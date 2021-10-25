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
#include "FreeRTOS.h"
#include "task.h"
#include "pinetime_board.h"
#include "sys.h"
#include "backlight.h"

// ---------------------------------------------------------------------------------------------------------

static void on_error( uint8_t err ) {

    //NRF_BREAKPOINT_COND;

    //NVIC_SystemReset();    
    
    for (;;)
    {
        // show error blinking backlight n times
        // after 10 times, reset
        for (uint8_t ii = 0; ii < 10; ii++) {
            for (uint8_t i = 0; i < err; i++)
            {
                nrf_delay_ms(200);
                set_backlight_level(0);
                nrf_delay_ms(200);
                set_backlight_level(3);
            }
            
            nrf_delay_ms(1000);
        }
        NVIC_SystemReset();
    }
}

void assert_nrf_callback(uint16_t line_num, const uint8_t* p_file_name) {
    //NRF_LOG_ERROR("%s:%d", p_file_name, line_num);
    on_error(1);
}

void app_error_handler(uint32_t error_code, uint32_t line_num, const uint8_t * p_file_name) {
    //NRF_LOG_ERROR("%s:%d", p_file_name, line_num);
    //pinetimecos.debug = error_code;
    on_error(2);
}


void app_error_fault_handler(uint32_t id, uint32_t pc, uint32_t info) {
    //NRF_LOG_ERROR("Received a fault! id: 0x%08x, pc: 0x%08x, info: 0x%08x", id, pc, info);
    on_error(3);
}


void app_error_handler_bare(uint32_t error_code) {
    // Local APP_ERROR_CHECK

    //NRF_LOG_ERROR("Received an error: 0x%08x!", error_code);
    pinetimecos.debug = error_code;
    //on_error(4);
}

// ---------------------------------------------------------------------------------------------------------

static void clock_init(void) {
    ret_code_t err_code = nrf_drv_clock_init();
    APP_ERROR_CHECK(err_code);
    nrf_drv_clock_lfclk_request(NULL);
}


static void log_init(void) {
    ret_code_t err_code = NRF_LOG_INIT(NULL);
    APP_ERROR_CHECK(err_code);

    //NRF_LOG_DEFAULT_BACKENDS_INIT();
}


// ---------------------------------------------------------------------------------------------------------

int main(void) {

    // WDT problems on init from bootloader
    NRF_WDT->RR[0] = WDT_RR_RR_Reload;
    NRF_WDT->TASKS_START = 0;

    //log_init();
    clock_init();

    sys_init();

    // Start FreeRTOS scheduler.
    vTaskStartScheduler();

    for (;;)
    {
        APP_ERROR_HANDLER(NRF_ERROR_FORBIDDEN);
    }
}


