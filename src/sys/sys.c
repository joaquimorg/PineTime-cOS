#include "sys.h"

#include "nrf_gpio.h"
#include "nrf_drv_gpiote.h"
#include "pinetime_board.h"
#include "lvgl.h"
#include "lvgl_init.h"
#include "nrf_ble.h"
#include "app.h"
#include "watchdog.h"
#include "rtc.h"
#include "backlight.h"

/* FreeRTOS related */
#include "FreeRTOS.h"
#include "task.h"
#include "timers.h"
#include "semphr.h"


#define SYS_TASK_DELAY          1
#define SYS_TASK_DELAY_SLEEP    100

TaskHandle_t  sys_task_handle;
TaskHandle_t  app_task_handle;
TimerHandle_t buttonTimer;


bool battery_is_powered(void){

    return nrf_gpio_pin_read(CHARGE_BASE_IRQ) ? false : true;
}

bool battery_is_charging(void) {

    return nrf_gpio_pin_read(CHARGE_IRQ) ? false : true;
}

void display_off(void) {
    
}

void display_on(void) {
    
}

static void _wdt_kick() {    

    if (nrf_gpio_pin_read(KEY_ACTION)) {
        return;
    }
    
    feed_watchdog();
}


void button_timer_callback(TimerHandle_t xTimer) {
    xTimerStop(xTimer, 0);
    if(pinetimecos.state == Running) {
        set_backlight_level(0);
        pinetimecos.state = Sleep;
    } else {
        pinetimecos.state = Running;
        set_backlight_level(pinetimecos.backlightValue);
    }
}

static void gpiote_pin_handler(nrf_drv_gpiote_pin_t pin, nrf_gpiote_polarity_t action) {

    if(pin == KEY_ACTION && action == NRF_GPIOTE_POLARITY_LOTOHI) {        
        BaseType_t xHigherPriorityTaskWoken = pdFALSE;
        xTimerStartFromISR(buttonTimer, &xHigherPriorityTaskWoken);
        portYIELD_FROM_ISR(xHigherPriorityTaskWoken);
        return ;
    }
    
}

static void sys_task_function(void* pvParameter) {

    ret_code_t err_code;

    UNUSED_PARAMETER(pvParameter);

    err_code = nrf_drv_gpiote_init();
    APP_ERROR_CHECK(err_code);

    backlight_init();
    set_backlight_level(2);

    init_watchdog();
    rtc_init();

    // Button
    buttonTimer = xTimerCreate ("buttonTimer", 200, pdFALSE, (void *) 0, button_timer_callback);
    nrf_gpio_cfg_output(KEY_ENABLE);
    nrf_gpio_pin_set(KEY_ENABLE);

    nrf_drv_gpiote_in_config_t in_config = GPIOTE_CONFIG_IN_SENSE_LOTOHI(true);
    in_config.pull = NRF_GPIO_PIN_PULLDOWN;
    err_code = nrf_drv_gpiote_in_init(KEY_ACTION, &in_config, gpiote_pin_handler);
    APP_ERROR_CHECK(err_code);
    nrf_drv_gpiote_in_event_enable(KEY_ACTION, true);

    pinetimecos.appTask = xTaskCreate(main_app, "APP", configMINIMAL_STACK_SIZE + 600, NULL, 3, &app_task_handle);
    pinetimecos.state = Running;

    while (true)
    {
        if(pinetimecos.state == Running) {
            vTaskDelay(SYS_TASK_DELAY);
            lv_timer_handler();
        } else {
            vTaskDelay(SYS_TASK_DELAY_SLEEP);
        }
        
        _wdt_kick();
        /* Tasks must be implemented to never return... */
    }
}


void sys_init(void) {
    
    lvgl_init();

    UNUSED_VARIABLE(xTaskCreate(sys_task_function, "SYS", configMINIMAL_STACK_SIZE + 200, NULL, 3, &sys_task_handle));

    nrf_ble_init();

}