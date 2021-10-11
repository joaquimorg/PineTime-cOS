#include "sys.h"

#include "nrf_gpio.h"
#include "nrf_drv_gpiote.h"
#include "pinetime_board.h"
#include "lvgl.h"
#include "lvgl_init.h"
#include "nrf_ble.h"
#include "utils.h"
#include "app.h"
#include "watchdog.h"
#include "rtc.h"
#include "backlight.h"
#include "st7789.h"

/* FreeRTOS related */
#include "FreeRTOS.h"
#include "task.h"
#include "timers.h"
#include "semphr.h"


#define SYS_TASK_DELAY          pdMS_TO_TICKS( 100 )
#define SYS_TASK_DELAY_SLEEP    pdMS_TO_TICKS( 1000 )

TimerHandle_t buttonTimer;

static void _wdt_kick() {    

    if (nrf_gpio_pin_read(KEY_ACTION)) {
        return;
    }    
    feed_watchdog();
}

void idle_timer_callback(TimerHandle_t xTimer) {
    app_push_message(Timeout);
}

void reload_idle_timer(void) {
  if(pinetimecos.state == Sleep) return;
  xTimerReset(idleTimer, 0);
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
        app_push_message(TouchPushed);
    }
    if(pin == CHARGE_IRQ && action == NRF_GPIOTE_POLARITY_TOGGLE) {
        if (nrf_gpio_pin_read(CHARGE_IRQ)) {
            pinetimecos.chargingState = StatusOFF;
        } else {
            pinetimecos.chargingState = StatusON;
        }
        //app_push_message(Charging);
    }

    if(pin == CHARGE_BASE_IRQ && action == NRF_GPIOTE_POLARITY_TOGGLE) {        
        if (nrf_gpio_pin_read(CHARGE_BASE_IRQ)) {
            pinetimecos.powerState = StatusOFF;
        } else {
            pinetimecos.powerState = StatusON;
        }
    }
    
}

void vApplicationTickHook(void) {
    if(pinetimecos.state == Running) { 
        //lv_tick_inc(LV_DISP_DEF_REFR_PERIOD);
        //lv_timer_handler();
    }
}


static void lvgl_task_function(void* pvParameter) { 

    UNUSED_PARAMETER(pvParameter);

    /*TickType_t xLastWakeTime; 
    const TickType_t xPeriod = pdMS_TO_TICKS( 1 ); 
    xLastWakeTime = xTaskGetTickCount(); */

    for(;;) { 
        //vTaskDelayUntil( &xLastWakeTime, xPeriod );        
        if(pinetimecos.state == Running) { 
            lv_tick_inc(15);
            lv_timer_handler();
        }
        vTaskDelay(pdMS_TO_TICKS( 1 ));
        _wdt_kick();
    } 

    vTaskDelete(NULL); 
}


static void sys_task_function(void* pvParameter) {

    UNUSED_PARAMETER(pvParameter);

    while (true)
    {
        if(pinetimecos.state == Running) {
            vTaskDelay(SYS_TASK_DELAY);
        } else {
            vTaskDelay(SYS_TASK_DELAY_SLEEP);
        }
        
    }
}


void sys_init(void) {

    pinetimecos.state = Sleep;
    pinetimecos.bluetoothState = StatusOFF;
    pinetimecos.chargingState = StatusOFF;
    pinetimecos.powerState = StatusOFF;

    nrf_drv_gpiote_init();
    
    init_watchdog();
    rtc_init();

    // Button
    buttonTimer = xTimerCreate ("buttonTimer", 200, pdFALSE, (void *) 0, button_timer_callback);
    nrf_gpio_cfg_output(KEY_ENABLE);
    nrf_gpio_pin_set(KEY_ENABLE);

    nrf_drv_gpiote_in_config_t key_config = GPIOTE_CONFIG_IN_SENSE_LOTOHI(true);
    key_config.pull = NRF_GPIO_PIN_PULLDOWN;
    nrf_drv_gpiote_in_init(KEY_ACTION, &key_config, gpiote_pin_handler);
    nrf_drv_gpiote_in_event_enable(KEY_ACTION, true);

    // Touch irq
    nrf_drv_gpiote_in_config_t tp_config = GPIOTE_CONFIG_IN_SENSE_HITOLO(true);
    tp_config.pull = NRF_GPIO_PIN_PULLUP;
    nrf_drv_gpiote_in_init(TP_IRQ, &tp_config, gpiote_pin_handler);
    nrf_drv_gpiote_in_event_enable(TP_IRQ, true);

    // CHARGE_IRQ
    nrf_drv_gpiote_in_config_t ch_config = GPIOTE_CONFIG_IN_SENSE_TOGGLE(true);
    //tp_config.pull = NRF_GPIO_PIN_PULLDOWN;
    nrf_drv_gpiote_in_init(CHARGE_IRQ, &ch_config, gpiote_pin_handler);
    nrf_drv_gpiote_in_event_enable(CHARGE_IRQ, true);

    // CHARGE_BASE_IRQ
    nrf_drv_gpiote_in_config_t cb_config = GPIOTE_CONFIG_IN_SENSE_TOGGLE(true);
    //tp_config.pull = NRF_GPIO_PIN_PULLDOWN;
    nrf_drv_gpiote_in_init(CHARGE_BASE_IRQ, &cb_config, gpiote_pin_handler);
    nrf_drv_gpiote_in_event_enable(CHARGE_BASE_IRQ, true);

    lvgl_init();
    backlight_init();
    set_backlight_level(2);

    UNUSED_VARIABLE(xTaskCreate(lvgl_task_function, "LVGL", 256, NULL, 5, (TaskHandle_t *) NULL )); 
    UNUSED_VARIABLE(xTaskCreate(sys_task_function, "SYS", 256, NULL, 6, (TaskHandle_t *) NULL));
    pinetimecos.appTask = xTaskCreate(main_app, "APP", 1024, NULL, 6, (TaskHandle_t *) NULL);

    idleTimer = xTimerCreate ("idleTimer", pdMS_TO_TICKS(30000), pdFALSE, NULL, idle_timer_callback);
    xTimerStart(idleTimer, 0);

    nrf_ble_init();

}
