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
#include "watchdog.h"
#include "rtc.h"
#include "backlight.h"
#include "st7789.h"


void display_off(void) {
    xTimerStop(idleTimer, 0);
    set_backlight_level(0);
    st7789_display_off();
    pinetimecos.state = Sleep;    
}

void display_on(void) {
    pinetimecos.state = Running;
    xTimerStart(idleTimer, 0);
    st7789_display_on();
    set_backlight_level(pinetimecos.backlightValue);
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