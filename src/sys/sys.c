#include "sys.h"

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

/* FreeRTOS related */
#include "FreeRTOS.h"
#include "task.h"
#include "timers.h"
#include "semphr.h"


#define SYS_TASK_DELAY          pdMS_TO_TICKS( 250 )
#define SYS_TASK_DELAY_SLEEP    pdMS_TO_TICKS( 1000 )

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
    app_push_message(ButtonPushed);
}

static void gpiote_pin_handler(nrf_drv_gpiote_pin_t pin, nrf_gpiote_polarity_t action) {

    if(pin == KEY_ACTION && action == NRF_GPIOTE_POLARITY_LOTOHI) {        
        BaseType_t xHigherPriorityTaskWoken = pdFALSE;
        xTimerStartFromISR(buttonTimer, &xHigherPriorityTaskWoken);
        portYIELD_FROM_ISR(xHigherPriorityTaskWoken);
        return ;
    }
    if (pin == TP_IRQ && action == NRF_GPIOTE_POLARITY_HITOLO) {
        if(pinetimecos.state == Sleep) {
            cst816Get();
            if ( tsData.gesture == TOUCH_DOUBLE_CLICK ) {
                display_on();
            }
        }
        return ;
    }
    
}

void vApplicationTickHook(void) {

}


static void lvgl_task_function(void* pvParameter) { 

    UNUSED_PARAMETER(pvParameter);

    while (true) {
        
        if(pinetimecos.state == Running) { 
            if (pinetimecos.lvglstate == Running) {
                lv_tick_inc(5);
                //lv_timer_handler();
            }
            
        }
        vTaskDelay(pdMS_TO_TICKS( 2 ));
        
    } 

    vTaskDelete(NULL); 
}


static void sys_task_function(void* pvParameter) {

    UNUSED_PARAMETER(pvParameter);
    xTimerStart(idleTimer, 0);

    while (true) {

        if(pinetimecos.state == Running) {
            vTaskDelay(SYS_TASK_DELAY);
        } else {
            vTaskDelay(SYS_TASK_DELAY_SLEEP);
        }

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
   

    }
    vTaskDelete(NULL); 
}


void sys_init(void) {

    pinetimecos.state = Sleep;
    pinetimecos.bluetoothState = StatusOFF;
    pinetimecos.chargingState = StatusOFF;
    pinetimecos.powerState = StatusOFF;

    pinetimecos.debug = 0;

    pinetimecos.batteryVoltage = 0.0f;
    pinetimecos.batteryPercentRemaining = -1;

    pinetimecos.displayTimeout = 60000;

    nrf_drv_gpiote_init();
        
    rtc_init();    

    // Button
    buttonTimer = xTimerCreate ("buttonTimer", 300, pdFALSE, (void *) 0, button_timer_callback);
    nrf_gpio_cfg_output(KEY_ENABLE);
    nrf_gpio_pin_set(KEY_ENABLE);

    nrf_drv_gpiote_in_config_t key_config = GPIOTE_CONFIG_IN_SENSE_LOTOHI(false);
    key_config.pull = NRF_GPIO_PIN_PULLDOWN;
    nrf_drv_gpiote_in_init(KEY_ACTION, &key_config, gpiote_pin_handler);
    nrf_drv_gpiote_in_event_enable(KEY_ACTION, true);

    // Touch irq
    nrf_drv_gpiote_in_config_t tp_config = GPIOTE_CONFIG_IN_SENSE_HITOLO(false);
    tp_config.pull = NRF_GPIO_PIN_PULLUP;
    nrf_drv_gpiote_in_init(TP_IRQ, &tp_config, gpiote_pin_handler);
    nrf_drv_gpiote_in_event_enable(TP_IRQ, true);

    // CHARGE_IRQ
    nrf_gpio_cfg_input(CHARGE_IRQ, NRF_GPIO_PIN_NOPULL);

    // CHARGE_BASE_IRQ
    nrf_gpio_cfg_input(CHARGE_BASE_IRQ, NRF_GPIO_PIN_NOPULL);

    spiflash_init();

    nrf_ble_init();

    lvgl_init();
    backlight_init();
    set_backlight_level(2);

    battery_init();
    
    UNUSED_VARIABLE(xTaskCreate(sys_task_function, "SYS", configMINIMAL_STACK_SIZE, NULL, 2, (TaskHandle_t *) NULL));
    UNUSED_VARIABLE(xTaskCreate(lvgl_task_function, "LVGL", configMINIMAL_STACK_SIZE, NULL, 2, (TaskHandle_t *) NULL));
    UNUSED_VARIABLE(xTaskCreate(main_app, "APP", configMINIMAL_STACK_SIZE + 512, NULL, 2, (TaskHandle_t *) NULL));

    idleTimer = xTimerCreate ("idleTimer", pdMS_TO_TICKS(pinetimecos.displayTimeout), pdFALSE, NULL, idle_timer_callback);

    bleTimer = xTimerCreate ("bleTimer", pdMS_TO_TICKS(30000), pdTRUE, NULL, ble_timer_callback);
    
}
