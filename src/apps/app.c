#include "nordic_common.h"
#include "nrf_delay.h"
#include "nrf_gpio.h"

/* FreeRTOS related */
#include "FreeRTOS.h"
#include "task.h"
#include "semphr.h"

#include "app.h"
#include "sys.h"
#include "utils.h"
#include "lvgl.h"
#include "rtc.h"
#include "cst816.h"
#include "watchdog.h"
#include "pinetime_board.h"

#include "clock.h"
#include "info.h"


#define APP_TASK_DELAY          pdMS_TO_TICKS( 5 )
#define APP_TASK_DELAY_SLEEP    pdMS_TO_TICKS( 500 )

void app_push_message(enum appMessages msg);

static void _wdt_kick() {    

    if (nrf_gpio_pin_read(KEY_ACTION)) {
        return;
    }    
    feed_watchdog();
}

static void gesture_event_cb(lv_event_t * e) {

    switch (tsData.gesture) {
        case TOUCH_SLIDE_LEFT:
            pinetimecosapp.gestureDir = DIR_LEFT;
            break;
        case TOUCH_SLIDE_RIGHT:
            pinetimecosapp.gestureDir = DIR_RIGHT;
            break;
        case TOUCH_SLIDE_UP:
            pinetimecosapp.gestureDir = DIR_TOP;
            break;
        case TOUCH_SLIDE_DOWN:
            pinetimecosapp.gestureDir = DIR_BOTTOM;
            break;
        default:
            pinetimecosapp.gestureDir = DIR_NONE;
            break;
    }
    tsData.gesture = TOUCH_NO_GESTURE;
   
    if ( pinetimecosapp.gestureDir != DIR_NONE ) {
        app_push_message(Gesture);
    }
}

lv_timer_t * app_timer;

int app_init(app_t *app, lv_obj_t * parent) {
    return app->spec->init(app, parent);
}

int app_update(app_t *app) {
    return app->spec->update(app);
}

int app_close(app_t *app) {
    return app->spec->close(app);
}


void set_refresh_direction(enum RefreshDirections dir) {
    pinetimecosapp.refreshDirection = dir;
    if ( dir == Up ) {
        lv_disp_set_direction(lv_disp_get_default(), 0);
    } else {
        lv_disp_set_direction(lv_disp_get_default(), 1);
    }
}

void load_app(app_t *app, lv_obj_t * parent) {
    if (app == pinetimecosapp.active_app) {
        return;
    }
    pinetimecos.lvglstate = Sleep;
    lv_timer_pause(app_timer);
    if (pinetimecosapp.active_app) {
        UNUSED_VARIABLE(app_close(pinetimecosapp.active_app));
    }

    pinetimecosapp.active_app = app;
    UNUSED_VARIABLE(app_init(pinetimecosapp.active_app, parent));    

    pinetimecos.lvglstate = Running;
    lv_timer_resume(app_timer);    
}

void update_time(lv_timer_t * timer) {
    if (pinetimecosapp.active_app) {
        UNUSED_VARIABLE(app_update(pinetimecosapp.active_app));
    }
}

void main_app(void* pvParameter) {
    UNUSED_PARAMETER(pvParameter); 

    enum appMessages msg;
    appMsgQueue = xQueueCreate(QUEUESIZE, ITEMSIZE);

    lv_obj_t *scr = lv_scr_act();

    lv_obj_add_event_cb(scr, gesture_event_cb, LV_EVENT_PRESSING, NULL);

    set_refresh_direction(Up);
    load_app(APP_CLOCK, scr);

    app_timer = lv_timer_create(update_time, 250, NULL);

    xTaskNotifyGive(xTaskGetCurrentTaskHandle());

    pinetimecos.state = Running;

    init_watchdog();

    while (true) {
   
        if (xQueueReceive(appMsgQueue, &msg, ( TickType_t ) APP_TASK_DELAY)) {
            switch (msg)
            {
                case Timeout:
                case ButtonPushed:
                    if(pinetimecos.state == Running) {
                        display_off();
                    } else {
                        display_on();
                    }
                    break;
                case WakeUp:
                    if(pinetimecos.state == Sleep) {
                        display_on();
                    }
                    break;
                case TouchPushed:
                    reload_idle_timer();
                    break;
                case Gesture:
                    switch (pinetimecosapp.gestureDir) {
                        case DIR_TOP:
                            if (pinetimecosapp.active_app == APP_INFO ) {
                                set_refresh_direction(Up);
                                load_app(APP_CLOCK, scr);
                            }
                            break;

                        case DIR_BOTTOM:
                            if (pinetimecosapp.active_app == APP_CLOCK ) {
                                set_refresh_direction(Down);
                                load_app(APP_INFO, scr);
                            }
                            break;
                        
                        default:
                            break;
                    }
                    pinetimecosapp.gestureDir = DIR_NONE;
                    //pinetimecos.debug = pinetimecos.gestureDir;
                    break;
                case UpdateBleConnection:
                    break;
                default:
                    break;
            }
        }
        if(pinetimecos.state != Running) {
            vTaskDelay(APP_TASK_DELAY_SLEEP);
        } else {
            if (pinetimecos.lvglstate == Running) {
                lv_timer_handler();
            }
        }

        _wdt_kick();
        /* Tasks must be implemented to never return... */
    }

    vTaskDelete(NULL); 

}

void app_push_message(enum appMessages msg) {
    if( appMsgQueue == NULL ) return;
    if (in_isr()) {
        BaseType_t xHigherPriorityTaskWoken;
        xHigherPriorityTaskWoken = pdFALSE;
        xQueueSendFromISR(appMsgQueue, &msg, &xHigherPriorityTaskWoken);
        if (xHigherPriorityTaskWoken) {
            /* Actual macro used here is port specific. */
            portYIELD_FROM_ISR(xHigherPriorityTaskWoken);
        }
    } else {
        xQueueSend(appMsgQueue, &msg, portMAX_DELAY);
    }
}