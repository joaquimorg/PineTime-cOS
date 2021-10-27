#include "nordic_common.h"
#include "nrf_delay.h"
#include "nrf_gpio.h"
#include "nrf_drv_gpiote.h"

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
#include "motor.h"
#include "watchdog.h"
#include "pinetime_board.h"

#include "clock.h"
#include "debug.h"
#include "menu.h"
#include "notification.h"


#define APP_TASK_DELAY_SLEEP    pdMS_TO_TICKS( 1000 )

static lv_obj_t *main_scr;
static lv_timer_t * app_timer;

void app_push_message(enum appMessages msg);
void load_application(enum apps app, enum RefreshDirections dir);

int app_init(app_t *app, lv_obj_t * parent) {
    return app->spec->init(app, parent);
}

int app_update(app_t *app) {
    return app->spec->update(app);
}

int app_gesture(app_t *app, enum appGestures gesture) {
    return app->spec->gesture(app, gesture);
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
    TickType_t queueTimeout = 1;

    appMsgQueue = xQueueCreate(QUEUESIZE, ITEMSIZE);
    pinetimecosapp.runningApp = NULL;

    main_scr = lv_scr_act();

    load_application(Clock, AnimUp);

    app_timer = lv_timer_create(update_time, 1000, NULL);

    xTaskNotifyGive(xTaskGetCurrentTaskHandle());

    xTimerStart(idleTimer, 0);

    nrf_drv_gpiote_in_event_enable(KEY_ACTION, true);
    nrf_drv_gpiote_in_event_enable(TP_IRQ, true);

    //motor_start(10);

    pinetimecos.lvglstate = Running;
    pinetimecos.state = Running;
    while (true) {

        if (xQueueReceive(appMsgQueue, &msg, queueTimeout) == pdPASS) {
            switch (msg) {

                case Timeout:
                    if(pinetimecos.state == Running) {
                        display_off();
                    }
                    break;

                case ButtonPushed:
                    if (pinetimecosapp.activeApp == Clock) {
                        display_off();
                    } else {
                        load_application(pinetimecosapp.returnApp, pinetimecosapp.returnAnimation);
                        reload_idle_timer();
                    }
                    break;

                case WakeUp:
                    if(pinetimecos.state == Sleep) {
                        display_on();
                    }
                    break;

                case Gesture:

                    if ( pinetimecosapp.gestureDir != DirNone ) {

                        if ( pinetimecosapp.gestureDir == pinetimecosapp.returnDir ) {
                            load_application(pinetimecosapp.returnApp, pinetimecosapp.returnAnimation);
                        } else {
                            // send gesture to app
                            UNUSED_VARIABLE(app_gesture(pinetimecosapp.runningApp, pinetimecosapp.gestureDir));
                        }

                        pinetimecosapp.gestureDir = DirNone;
                    }
                    break;

                case UpdateBleConnection:
                    break;

                case NewNotification:
                    if(pinetimecos.state == Sleep) {
                        display_on();
                    }
                    if ( pinetimecosapp.activeApp != Notification ) {
                        motor_start(10);
                        load_application(Notification, AnimUp);
                        reload_idle_timer();
                    }
                    break;

                default:
                    break;
            }
        }

        if(pinetimecos.state == Sleep) {
            vTaskDelay(APP_TASK_DELAY_SLEEP);
            //queueTimeout = portMAX_DELAY;
            //vTaskDelay(1);
        } else {
            //queueTimeout = 1;
            lv_timer_handler();
            lv_tick_inc(20);
        }
        _wdt_kick();
    }

    vTaskDelete(NULL);

}

static void return_app(enum apps app, enum appGestures gesture, enum RefreshDirections dir) {
    pinetimecosapp.returnApp = app;
    pinetimecosapp.returnAnimation = dir;
    pinetimecosapp.returnDir = gesture;
}

void load_application(enum apps app, enum RefreshDirections dir) {

    set_refresh_direction(dir);
    pinetimecosapp.activeApp = app;
    switch (app) {

        case Debug:
            run_app(APP_DEBUG);
            return_app(Menu, DirTop, AnimUp);
            break;

        case Clock:
            run_app(APP_CLOCK);
            return_app(Clock, DirNone, AnimNone);
            break;

        case Menu:
            run_app(APP_MENU);
            return_app(Clock, DirTop, AnimUp);
            break;

        case Notification:
            run_app(APP_NOTIFICATION);
            return_app(Clock, DirBottom, AnimDown);
            break;

        default:
            break;
    }
}


void app_push_message(enum appMessages msg) {
    if( appMsgQueue == NULL ) return;
    if (in_isr()) {
        if (xQueueIsQueueFullFromISR(appMsgQueue)) {
            return;
        }
        BaseType_t xHigherPriorityTaskWoken;
        xHigherPriorityTaskWoken = pdFALSE;
        xQueueSendFromISR(appMsgQueue, &msg, &xHigherPriorityTaskWoken);
        if (xHigherPriorityTaskWoken) {
            /* Actual macro used here is port specific. */
            //portYIELD_FROM_ISR(xHigherPriorityTaskWoken);
        }
    } else {
        xQueueSend(appMsgQueue, &msg, ( TickType_t ) 0);
    }
}