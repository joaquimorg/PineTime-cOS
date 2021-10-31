#include <stdint.h>
#include <string.h>
#include "nordic_common.h"
#include "nrf_drv_clock.h"
#include "nrf_gpio.h"
#include "nrf_delay.h"
#include "nrf_sdh_freertos.h"
#include "nrf_drv_power.h"
#include "app_error.h"
#include "app_error_weak.h"
#include "FreeRTOS.h"
#include "task.h"
#include "pinetime_board.h"
#include "sys.h"
#include "st7789.h"
#include "backlight.h"

// ---------------------------------------------------------------------------------------------------------

static void on_error( uint8_t err ) {

    //NRF_BREAKPOINT_COND;

    //NVIC_SystemReset();    
    
    st7789_vertical_scroll_definition(0, 320, 0, 0);
    draw_square(0, 0, 239, 239, RGB2COLOR(0xa0, 0xa0, 0xa0));

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
            
            nrf_delay_ms(2000);
        }
        NVIC_SystemReset();
    }
}

void assert_nrf_callback(uint16_t line_num, const uint8_t* p_file_name) {
    //NRF_LOG_ERROR("%s:%d", p_file_name, line_num);
    NRF_BREAKPOINT_COND;
    on_error(1);
}

void app_error_handler(uint32_t error_code, uint32_t line_num, const uint8_t * p_file_name) {
    //NRF_LOG_ERROR("%s:%d", p_file_name, line_num);
    //pinetimecos.debug = error_code;
    NRF_BREAKPOINT_COND;
    on_error(2);
}


void app_error_fault_handler(uint32_t id, uint32_t pc, uint32_t info) {
    //NRF_LOG_ERROR("Received a fault! id: 0x%08x, pc: 0x%08x, info: 0x%08x", id, pc, info);
    NRF_BREAKPOINT_COND;
    on_error(3);
}


void app_error_handler_bare(uint32_t error_code) {
    // Local APP_ERROR_CHECK

    //NRF_LOG_ERROR("Received an error: 0x%08x!", error_code);
    pinetimecos.debug = error_code;
    //on_error(error_code);
    NRF_BREAKPOINT_COND;
}

// ---------------------------------------------------------------------------------------------------------

static void clock_init(void) {
    ret_code_t err_code = nrf_drv_clock_init();
    APP_ERROR_CHECK(err_code);
    nrf_drv_clock_lfclk_request(NULL);
}


static void i2c_clean_up(void) {
    // Unblock i2c?
    nrf_gpio_cfg(TWI_SCL,
                NRF_GPIO_PIN_DIR_OUTPUT,
                NRF_GPIO_PIN_INPUT_DISCONNECT,
                NRF_GPIO_PIN_NOPULL,
                NRF_GPIO_PIN_S0D1,
                NRF_GPIO_PIN_NOSENSE);

    nrf_gpio_pin_set(TWI_SCL);

    for (uint8_t i = 0; i < 16; i++) {
        nrf_gpio_pin_toggle(TWI_SCL);
        nrf_delay_us(5);
    }

    nrf_gpio_cfg_default(TWI_SCL);
}

// ---------------------------------------------------------------------------------------------------------

void vApplicationTickHook(void) {
    
}

int main(void) {

    ret_code_t ret;

    NVIC_SetPriority(FPU_IRQn, APP_IRQ_PRIORITY_LOW);
    NVIC_EnableIRQ(FPU_IRQn);

    // WDT problems on init from bootloader
    //NRF_WDT->RR[0] = WDT_RR_RR_Reload;
    //NRF_WDT->TASKS_START = 0; 

    i2c_clean_up();
    
    ret = nrf_drv_power_init(NULL);
    APP_ERROR_CHECK(ret);

    //log_init();
    clock_init();

    sys_init();

    // Activate deep sleep mode.
    SCB->SCR |= SCB_SCR_SLEEPDEEP_Msk;
    
    // Start FreeRTOS scheduler.
    vTaskStartScheduler();

    for (;;)
    {
        APP_ERROR_HANDLER(NRF_ERROR_FORBIDDEN);
    }
}


