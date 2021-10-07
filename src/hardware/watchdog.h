#ifndef WATCHDOG_CONFIG_H
#define WATCHDOG_CONFIG_H

#include "nordic_common.h"
#include "nrf_drv_wdt.h"

#define   WTD_TIMEOUT 10000

nrf_drv_wdt_channel_id wtd_channel_id;

void init_watchdog(void);
void feed_watchdog(void);


#endif //WATCHDOG_CONFIG_H