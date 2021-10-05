#include <stdint.h>
#include <string.h>
#include "nordic_common.h"
#include "nrf_drv_clock.h"
#include "nrf_gpio.h"
#include "nrf_delay.h"
#include "nrf_sdh_freertos.h"
#include "nrf_log.h"
#include "nrf_log_ctrl.h"
#include "nrf_log_default_backends.h"
#include "app_error.h"
#include "app_error_weak.h"

#include "nrf_ble.h"

/* FreeRTOS related */
#include "FreeRTOS.h"
#include "task.h"
#include "semphr.h"

#include "pinetime_board.h"
#include "lvgl.h"
#include "lvgl_init.h"
#include "app.h"

#define SYS_TASK_DELAY        1 
TaskHandle_t  sys_task_handle;
TaskHandle_t  app_task_handle;

// ---------------------------------------------------------------------------------------------------------

static void on_error( uint8_t err ) {

    //NRF_BREAKPOINT_COND;

    //NVIC_SystemReset();
    
    for (;;)
    {
        for (uint8_t i = 0; i < err; i++)
        {
            nrf_delay_ms(200);
            nrf_gpio_pin_clear(LCD_LIGHT_1);
            nrf_delay_ms(200);
            nrf_gpio_pin_set(LCD_LIGHT_1);
        }
        nrf_delay_ms(1000);
    }
}

void assert_nrf_callback(uint16_t line_num, const uint8_t* p_file_name) {
    NRF_LOG_ERROR("%s:%d", p_file_name, line_num);
    on_error(1);
}

void app_error_handler(uint32_t error_code, uint32_t line_num, const uint8_t * p_file_name) {
    NRF_LOG_ERROR("%s:%d", p_file_name, line_num);
    on_error(2);
}


void app_error_fault_handler(uint32_t id, uint32_t pc, uint32_t info) {
    NRF_LOG_ERROR("Received a fault! id: 0x%08x, pc: 0x%08x, info: 0x%08x", id, pc, info);
    on_error(3);
}


void app_error_handler_bare(uint32_t error_code) {
    NRF_LOG_ERROR("Received an error: 0x%08x!", error_code);
    on_error(4);
}

// ---------------------------------------------------------------------------------------------------------

static void sys_task_function(void* pvParameter)
{
    UNUSED_PARAMETER(pvParameter);

    UNUSED_VARIABLE(xTaskCreate(main_app, "APP", configMINIMAL_STACK_SIZE + 600, NULL, 3, &app_task_handle));

    while (true)
    {
        vTaskDelay(SYS_TASK_DELAY);
        lv_tick_inc(SYS_TASK_DELAY);
        lv_timer_handler();
        
        /* Tasks must be implemented to never return... */
    }
}

void vApplicationIdleHook(void) {

}

static void clock_init(void) {
    ret_code_t err_code = nrf_drv_clock_init();
    APP_ERROR_CHECK(err_code);
}


static void log_init(void) {
    ret_code_t err_code = NRF_LOG_INIT(NULL);
    APP_ERROR_CHECK(err_code);

    NRF_LOG_DEFAULT_BACKENDS_INIT();
}


// ---------------------------------------------------------------------------------------------------------

int main(void)
{
    log_init();
    clock_init();

    lvgl_init();

    UNUSED_VARIABLE(xTaskCreate(sys_task_function, "SYS", configMINIMAL_STACK_SIZE + 200, NULL, 3, &sys_task_handle));

    nrf_ble_init();

    // Start FreeRTOS scheduler.
    vTaskStartScheduler();

    for (;;)
    {
        APP_ERROR_HANDLER(NRF_ERROR_FORBIDDEN);
    }
}


