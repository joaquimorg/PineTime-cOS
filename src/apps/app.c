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
static void load_application(enum apps app, enum RefreshDirections dir);

static void _wdt_kick() {    

    if (nrf_gpio_pin_read(KEY_ACTION)) {
        return;
    }    
    feed_watchdog();
}

static void gesture_event_cb(lv_event_t * e) {    

    switch (tsData.gesture) {
        case TOUCH_SLIDE_LEFT:
            pinetimecosapp.gestureDir = DirLeft;
            break;
        case TOUCH_SLIDE_RIGHT:
            pinetimecosapp.gestureDir = DirRight;
            break;
        case TOUCH_SLIDE_UP:
            pinetimecosapp.gestureDir = DirTop;
            break;
        case TOUCH_SLIDE_DOWN:
            pinetimecosapp.gestureDir = DirBottom;
            break;
        default:
            pinetimecosapp.gestureDir = DirNone;
            break;
    }
    tsData.gesture = TOUCH_NO_GESTURE;
   
    if ( pinetimecosapp.gestureDir != DirNone ) {
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
    if ( dir == AnimUp ) {
        lv_disp_set_direction(lv_disp_get_default(), 0);
    } else if ( dir == AnimDown ) {
        lv_disp_set_direction(lv_disp_get_default(), 1);
    } else if ( dir == AnimLeft ) {
        lv_disp_set_direction(lv_disp_get_default(), 2);
    } else if ( dir == AnimRight ) {
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

    load_application(Clock, AnimUp);

    lv_obj_add_event_cb(main_scr, gesture_event_cb, LV_EVENT_PRESSING, NULL);    

    app_timer = lv_timer_create(update_time, 1000, NULL);    

    //xTaskNotifyGive(xTaskGetCurrentTaskHandle());
    init_watchdog();
    
    pinetimecos.lvglstate = Running;
    pinetimecos.state = Running;
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
                            load_application(pinetimecosapp.returnApp, pinetimecosapp.returnAnimation);
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

                    if ( pinetimecosapp.activeApp == Clock ) {
                        switch (pinetimecosapp.gestureDir) {
                            case DirBottom:
                                load_application(Info, AnimDown);
                                break;
                            case DirRight:
                                load_application(Menu, AnimRight);
                                break;
                            default:
                                break;
                        }
                    } else if ( pinetimecosapp.gestureDir == pinetimecosapp.returnDir ) {
                        load_application(pinetimecosapp.returnApp, pinetimecosapp.returnAnimation);
                    }
                    
                    pinetimecosapp.gestureDir = DirNone;
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
                lv_tick_inc(20);
            }
        }

        _wdt_kick();
        /* Tasks must be implemented to never return... */
    }

    vTaskDelete(NULL); 

}


static void load_application(enum apps app, enum RefreshDirections dir) {

    set_refresh_direction(dir);
    switch (app) {

        case Clock:
            run_app(APP_CLOCK);
            pinetimecosapp.returnDir = DirNone;
            break;

        case Info:
            run_app(APP_INFO);
            pinetimecosapp.returnApp = Clock;
            pinetimecosapp.returnDir = DirTop;
            pinetimecosapp.returnAnimation = AnimUp;
            break;
        case Menu:
            run_app(APP_MENU);
            pinetimecosapp.returnApp = Clock;
            pinetimecosapp.returnDir = DirLeft;
            pinetimecosapp.returnAnimation = AnimLeft;
            break;

        default:
            break;
    }
    pinetimecosapp.activeApp = app;
}


void app_push_message(enum appMessages msg) {
    if( appMsgQueue == NULL ) return;
    if (in_isr()) {
        BaseType_t xHigherPriorityTaskWoken;
        xHigherPriorityTaskWoken = pdFALSE;
        xQueueSendFromISR(appMsgQueue, &msg, &xHigherPriorityTaskWoken);
        if (xHigherPriorityTaskWoken) {
            /* Actual macro used here is port specific. */
            //portYIELD_FROM_ISR(xHigherPriorityTaskWoken);
        }
    } else {
        xQueueSend(appMsgQueue, &msg, portMAX_DELAY);
    }
}