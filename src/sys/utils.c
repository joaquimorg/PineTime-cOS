#include <stdbool.h>
#include <stdint.h>

#include "utils.h"
#include "sys.h"
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
    xTimerStop(idleTimer, 0);
    set_backlight_level(0);
    st7789_sleep();
    pinetimecos.state = Sleep;
    //vTaskSuspend(pinetimecos.lvglTask);
}

void display_on(void) {
    pinetimecos.state = Running;
    //xTaskResumeFromISR(pinetimecos.lvglTask);
    xTimerStart(idleTimer, 0);
    st7789_wake_up();
    set_backlight_level(pinetimecos.backlightValue);
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

// -----------------------------------------------------------------------------------------

void ble_command(uint8_t msg_type) {
    uint32_t gTimestamp;
    
    switch (msg_type) {
        case COMMAND_TIME_UPDATE:

            gTimestamp = 
                (inputBuffer[3]) |
                (inputBuffer[2]) << 8 |
                (inputBuffer[1]) << 16 |
                (inputBuffer[0]) << 24;

            rtc_set_time(gTimestamp);
            break;
        
        case COMMAND_NOTIFICATION:
        case COMMAND_WEATHER:
            app_push_message(WakeUp);
            break;
        default:
            break;
    }

}