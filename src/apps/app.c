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
#include "menu.h"


#define APP_TASK_DELAY          pdMS_TO_TICKS( 5 )
#define APP_TASK_DELAY_SLEEP    pdMS_TO_TICKS( 500 )

static lv_obj_t *main_scr;
static lv_timer_t * app_timer;

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

int app_init(app_t *app, lv_obj_t * parent) {
    return app->spec->init(app, parent);
}

int app_update(app_t *app) {
    return app->spec->update(app);
}

int app_close(app_t *app) {
    return app->spec->close(app);
}


static void set_refresh_direction(enum RefreshDirections dir) {
    pinetimecosapp.refreshDirection = dir;
    if ( dir == Up ) {
        lv_disp_set_direction(lv_disp_get_default(), 0);
    } else if ( dir == Down ) {
        lv_disp_set_direction(lv_disp_get_default(), 1);
    } else if ( dir == Left ) {
        lv_disp_set_direction(lv_disp_get_default(), 2);
    } else if ( dir == Right ) {
        lv_disp_set_direction(lv_disp_get_default(), 3);
    }
}

static void run_app(app_t *app) {
    if (app == pinetimecosapp.runningApp) {
        return;
    }
    pinetimecos.lvglstate = Sleep;
    lv_timer_pause(app_timer);
    if (pinetimecosapp.runningApp) {
        UNUSED_VARIABLE(app_close(pinetimecosapp.runningApp));
    }
    pinetimecosapp.runningApp = app;
    UNUSED_VARIABLE(app_init(pinetimecosapp.runningApp, main_scr));
    lv_timer_set_period(app_timer, pinetimecosapp.runningApp->spec->updateInterval);

    pinetimecos.lvglstate = Running;
    lv_timer_resume(app_timer);    
}


static void load_application(enum apps app, enum RefreshDirections dir) {

    set_refresh_direction(dir);
    pinetimecosapp.returnDirection = dir;
    switch (app) {

        case Clock:
            run_app(APP_CLOCK);
            pinetimecosapp.returnDir = DIR_NONE;
            break;

        case Info:
            run_app(APP_INFO);
            pinetimecosapp.returnApp = Clock;
            pinetimecosapp.returnDir = DIR_TOP;
            pinetimecosapp.returnDirection = Up;
            break;
        case Menu:
            run_app(APP_MENU);
            pinetimecosapp.returnApp = Clock;
            pinetimecosapp.returnDir = DIR_LEFT;
            pinetimecosapp.returnDirection = Left;
            break;

        default:
            break;
    }
    pinetimecosapp.activeApp = app;
}

static void update_time(lv_timer_t * timer) {
    if (pinetimecosapp.runningApp) {
        UNUSED_VARIABLE(app_update(pinetimecosapp.runningApp));
    }
}

void main_app(void* pvParameter) {
    UNUSED_PARAMETER(pvParameter); 

    enum appMessages msg;
    appMsgQueue = xQueueCreate(QUEUESIZE, ITEMSIZE);
    pinetimecosapp.runningApp = NULL;
    
    main_scr = lv_scr_act();

    lv_obj_add_event_cb(main_scr, gesture_event_cb, LV_EVENT_PRESSING, NULL);    

    app_timer = lv_timer_create(update_time, 1000, NULL);

    load_application(Clock, Up);

    //xTaskNotifyGive(xTaskGetCurrentTaskHandle());

    init_watchdog();
    pinetimecos.state = Running;
    pinetimecos.lvglstate = Running;
    while (true) {
   
        if (xQueueReceive(appMsgQueue, &msg, ( TickType_t ) APP_TASK_DELAY)) {
            switch (msg)
            {
                case Timeout:
                    if(pinetimecos.state == Running) {
                        display_off();
                    }
                    break;

                case ButtonPushed:
                    if(pinetimecos.state == Running) {
                        if (pinetimecosapp.activeApp == Clock) {
                            display_off();
                        } else {
                            load_application(pinetimecosapp.returnApp, pinetimecosapp.returnDirection);
                        }
                    } else {
                        display_on();
                    }
                    break;

                case WakeUp:
                    if(pinetimecos.state == Sleep) {
                        display_on();
                    }
                    break;

                case Gesture:

                    if ( pinetimecosapp.gestureDir == pinetimecosapp.returnDir ) {
                        load_application(pinetimecosapp.returnApp, pinetimecosapp.returnDirection);
                    } else if ( pinetimecosapp.activeApp == Clock ) {
                        switch (pinetimecosapp.gestureDir) {
                            case DIR_BOTTOM:
                                load_application(Info, Down);
                                break;
                            case DIR_RIGHT:
                                load_application(Menu, Right);
                                break;
                            default:
                                break;
                        }
                    }
                    
                    pinetimecosapp.gestureDir = DIR_NONE;
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