#include "sys.h"

#include "nrf.h"
#include "nrf_gpio.h"
#include "nrf_drv_gpiote.h"
#include "pinetime_board.h"
#include "lvgl.h"
#include "lvgl_init.h"
#include "nrf_ble.h"
#include "ble_cmd.h"
#include "utils.h"
#include "app.h"
#include "watchdog.h"
#include "rtc.h"
#include "backlight.h"
#include "st7789.h"
#include "battery.h"
#include "cst816.h"
#include "flash.h"
#include "motor.h"

/* FreeRTOS related */
#include "FreeRTOS.h"
#include "task.h"
#include "timers.h"
#include "semphr.h"


#define SYS_TASK_DELAY          pdMS_TO_TICKS( 250 )
#define SYS_TASK_DELAY_SLEEP    pdMS_TO_TICKS( 30000 )

TimerHandle_t buttonTimer;


void ble_timer_callback(TimerHandle_t xTimer) {
    ble_update();
}

void reload_idle_timer(void) {
  if(pinetimecos.state == Sleep) return;
  xTimerReset(idleTimer, 0);
}

void idle_timer_callback(TimerHandle_t xTimer) {
    if (lv_disp_get_inactive_time(NULL) > pinetimecos.displayTimeout) {
        app_push_message(Timeout);
    } else {
        reload_idle_timer();
    }
}

void button_timer_callback(TimerHandle_t xTimer) {
    xTimerStop(xTimer, 0);
    if(pinetimecos.state == Running) {
        app_push_message(ButtonPushed);
    } else {
        display_on();
    }
}

static void gpiote_pin_handler(nrf_drv_gpiote_pin_t pin, nrf_gpiote_polarity_t action) {    

    uint32_t pin_level;
    UNUSED_PARAMETER(pin_level);
    pin_level = nrf_gpio_pin_read(pin);

    if (pin == TP_IRQ && action == NRF_GPIOTE_POLARITY_HITOLO && pin_level == 0) {
        
        if(pinetimecos.state == Sleep) {
            cst816Get();
            if ( tsData.gesture == TOUCH_DOUBLE_CLICK ) {
                display_on();
            }
        } else {
            cst816Get();
            switch (tsData.gesture) {
                case TOUCH_SLIDE_LEFT:
                    pinetimecosapp.gestureDir = DirLeft;
                    break;
                case TOUCH_SLIDE_RIGHT:
                    pinetimecosapp.gestureDir = DirRight;
                    break;
                case TOUCH_SLIDE_UP:
                    pinetimecosapp.gestureDir = DirTop;
                    break;
                case TOUCH_SLIDE_DOWN:
                    pinetimecosapp.gestureDir = DirBottom;
                    break;
                case TOUCH_SINGLE_CLICK:
                    pinetimecosapp.gestureDir = DirClick;
                    break;
                default:
                    pinetimecosapp.gestureDir = DirNone;
                    break;
            }
            if ( pinetimecosapp.gestureDir != DirNone ) {
                app_push_message(Gesture);
                reload_idle_timer();
            }
        }
        return;
    }

    if(pin == KEY_ACTION && action == NRF_GPIOTE_POLARITY_LOTOHI && pin_level == 1) {        
        BaseType_t xHigherPriorityTaskWoken = pdFALSE;
        xTimerStartFromISR(buttonTimer, &xHigherPriorityTaskWoken);
        portYIELD_FROM_ISR(xHigherPriorityTaskWoken);        
        return;   
    }
    
}

static void sys_task_function(void* pvParameter) {

    UNUSED_PARAMETER(pvParameter);

    buttonTimer = xTimerCreate ("buttonTimer", 300, pdFALSE, NULL, button_timer_callback);
    idleTimer = xTimerCreate ("idleTimer", pdMS_TO_TICKS(pinetimecos.displayTimeout), pdFALSE, NULL, idle_timer_callback);
    bleTimer = xTimerCreate ("bleTimer", pdMS_TO_TICKS(30000), pdTRUE, NULL, ble_timer_callback);    

    lvgl_init();
    backlight_init();
    set_backlight_level(pinetimecos.backlightLevel);
    
    battery_init();
    battery_read();

    spiflash_init();

    motor_init();

    UNUSED_VARIABLE(xTaskCreate(main_app, "APP", configMINIMAL_STACK_SIZE + 1024, NULL, 2, (TaskHandle_t *) NULL));    
    
    while (true) {

        // Read Charge Pin State
        if (nrf_gpio_pin_read(CHARGE_IRQ)) {
            pinetimecos.chargingState = StatusOFF;
        } else {
            if (pinetimecos.chargingState == StatusOFF) {
                app_push_message(Charging);
                pinetimecos.chargingState = StatusON;
            }
        }
        
        // Read Base Pin State
        if (nrf_gpio_pin_read(CHARGE_BASE_IRQ)) {
            pinetimecos.powerState = StatusOFF;
        } else {
            if (pinetimecos.powerState == StatusOFF) {
                pinetimecos.powerState = StatusON;
            }
        }
        
        // Read battery voltage
        battery_read();

        // Read step counter

        // Control the HR reading 

        if(pinetimecos.state == Running) {
            vTaskDelay(SYS_TASK_DELAY);
        } else {
            vTaskDelay(SYS_TASK_DELAY_SLEEP);
        }

    }
    vTaskDelete(NULL); 
}


void sys_init(void) {

    rtc_init();

    // Initialize watchdog
    init_watchdog();

    nrf_drv_gpiote_init();

    pinetimecos.resetReason = actual_reset_reason();
    pinetimecos.state = Running;
    pinetimecos.bluetoothState = StatusOFF;
    pinetimecos.chargingState = StatusOFF;
    pinetimecos.powerState = StatusOFF;

    pinetimecos.backlightLevel = 2;

    pinetimecos.debug = 0;

    pinetimecos.batteryVoltage = 0.0f;
    pinetimecos.batteryPercentRemaining = -1;

    pinetimecos.displayTimeout = 30000;

    pinetimecosBLE.notificationCount = 0;

    pinetimecos.dontDisturb = true;

    // Button
    nrf_gpio_cfg_output(KEY_ENABLE);
    nrf_gpio_pin_set(KEY_ENABLE);

    nrf_gpio_cfg_sense_input(KEY_ACTION, NRF_GPIO_PIN_PULLDOWN, NRF_GPIO_PIN_SENSE_HIGH);
    nrf_drv_gpiote_in_config_t key_config = GPIOTE_CONFIG_IN_SENSE_LOTOHI(false);
    key_config.pull = NRF_GPIO_PIN_PULLDOWN;
    key_config.skip_gpio_setup = true;
    nrf_drv_gpiote_in_init(KEY_ACTION, &key_config, gpiote_pin_handler);    
    
    // Touch irq
    nrf_gpio_cfg_sense_input(TP_IRQ, NRF_GPIO_PIN_PULLUP, NRF_GPIO_PIN_SENSE_LOW);
    nrf_drv_gpiote_in_config_t tp_config = GPIOTE_CONFIG_IN_SENSE_HITOLO(false);
    tp_config.pull = NRF_GPIO_PIN_PULLUP;
    tp_config.skip_gpio_setup = true;
    nrf_drv_gpiote_in_init(TP_IRQ, &tp_config, gpiote_pin_handler);
    
    // CHARGE_IRQ
    nrf_gpio_cfg_input(CHARGE_IRQ, NRF_GPIO_PIN_NOPULL);

    // CHARGE_BASE_IRQ
    nrf_gpio_cfg_input(CHARGE_BASE_IRQ, NRF_GPIO_PIN_NOPULL);

    nrf_ble_init();

    UNUSED_VARIABLE(xTaskCreate(sys_task_function, "SYS", configMINIMAL_STACK_SIZE + 256, NULL, 2, (TaskHandle_t *) NULL));
    //UNUSED_VARIABLE(xTaskCreate(main_app, "APP", configMINIMAL_STACK_SIZE + 512, NULL, 2, (TaskHandle_t *) NULL));
   
}
