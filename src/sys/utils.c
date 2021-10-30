#include <stdbool.h>
#include <stdint.h>

#include "utils.h"
#include "FreeRTOS.h"
#include "task.h"
#include "nrf_gpio.h"
#include "nrf_drv_gpiote.h"
#include "pinetime_board.h"
#include "nrf_ble.h"
#include "lvgl.h"
#include "app.h"
#include "sys.h"
#include "watchdog.h"
#include "rtc.h"
#include "backlight.h"
#include "st7789.h"


void display_off(void) {
    pinetimecos.state = Sleep;
    xTimerStop(idleTimer, 0);
    set_backlight_level(0);    
    st7789_sleep();
    sd_power_mode_set(NRF_POWER_MODE_LOWPWR);
}

void display_on(void) {
    xTimerStart(idleTimer, 0);
    st7789_wake_up();
    pinetimecos.state = Running;
    set_backlight_level(pinetimecos.backlightLevel);
    sd_power_mode_set(NRF_POWER_MODE_CONSTLAT);
}


void _wdt_kick() {

    if (nrf_gpio_pin_read(KEY_ACTION)) {
        return;
    }
    feed_watchdog();
}

// -----------------------------------------------------------------------------------------

const char* actual_reset_reason(void) {
  uint32_t reason = NRF_POWER->RESETREAS;
  NRF_POWER->RESETREAS = 0xffffffff;

  if (reason & 0x01u)
    return "Reset pin";
  if ((reason >> 1u) & 0x01u)
    return "Watchdog";
  if ((reason >> 2u) & 0x01u)
    return "Soft reset";
  if ((reason >> 3u) & 0x01u)
    return "CPU Lock-up";
  if ((reason >> 16u) & 0x01u)
    return "System OFF";
  if ((reason >> 17u) & 0x01u)
    return "LPCOMP";
  if ((reason) &0x01u)
    return "Debug interface";
  if ((reason >> 19u) & 0x01u)
    return "NFC";
  return "Hard reset";
}
