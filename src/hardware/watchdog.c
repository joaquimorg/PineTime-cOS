#include "watchdog.h"


void init_watchdog(void) {
    ret_code_t err_code;

    //Configure WDT.
    nrf_drv_wdt_config_t config = NRF_DRV_WDT_DEAFULT_CONFIG;

    err_code = nrf_drv_wdt_init(&config, NULL);
    APP_ERROR_CHECK(err_code);
    err_code = nrf_drv_wdt_channel_alloc(&wtd_channel_id);
    APP_ERROR_CHECK(err_code);
    nrf_drv_wdt_enable();

}

void feed_watchdog(void) {
    nrf_drv_wdt_channel_feed(wtd_channel_id);
}
