#include "nordic_common.h"
#include "nrf_delay.h"

/* FreeRTOS related */
#include "FreeRTOS.h"
#include "task.h"
#include "semphr.h"

#include "app.h"
#include "sys.h"
#include "utils.h"
#include "lvgl.h"
#include "rtc.h"

#include "clock.h"


#define APP_TASK_DELAY          pdMS_TO_TICKS( 5 )
#define APP_TASK_DELAY_SLEEP    pdMS_TO_TICKS( 100 )

lv_timer_t * app_timer;

int app_init(app_t *app) {
    return app->spec->init(app);
}

int app_update(app_t *app) {
    return app->spec->update(app);
}

int app_close(app_t *app) {
    return app->spec->close(app);
}

void load_app(app_t *app) {
    if (app == active_app) {
        return;
    }
    lv_timer_pause(app_timer);
    if (active_app) {
        UNUSED_VARIABLE(app_close(active_app));
    }
    active_app = app;
    UNUSED_VARIABLE(app_init(active_app));
    lv_timer_resume(app_timer);
}

void update_time(lv_timer_t * timer) {
    if (active_app) {
        UNUSED_VARIABLE(app_update(active_app));
    }
}

void main_app(void* pvParameter) {
    UNUSED_PARAMETER(pvParameter); 

    enum appMessages msg;
    appMsgQueue = xQueueCreate(QUEUESIZE, ITEMSIZE);

    load_app(APP_CLOCK);

    app_timer = lv_timer_create(update_time, 250, NULL);

    pinetimecos.state = Running;

    while (true) {
   
        if (xQueueReceive(appMsgQueue, &msg, ( TickType_t ) APP_TASK_DELAY)) {
            switch (msg)
            {
                case Timeout:
                case ButtonPushed:
                    if(pinetimecos.state == Running) {
                        display_off();
                        lv_timer_pause(app_timer);
                    } else {
                        display_on();
                        lv_timer_resume(app_timer);
                    }
                    break;
                case WakeUp:
                    if(pinetimecos.state == Sleep) {
                        display_on();
                        lv_timer_resume(app_timer);
                    }
                    break;
                case TouchPushed:
                    reload_idle_timer();
                    break;
                case UpdateBleConnection:
                    break;
                default:
                    break;
            }
        }
        if(pinetimecos.state != Running) {
            //vTaskDelayUntil( &xLastWakeTime, APP_TASK_DELAY_SLEEP );
            vTaskDelay(APP_TASK_DELAY_SLEEP);
        }
        /* Tasks must be implemented to never return... */
    }

}

void app_push_message(enum appMessages msg) {
  BaseType_t xHigherPriorityTaskWoken;
  xHigherPriorityTaskWoken = pdFALSE;
  xQueueSendFromISR(appMsgQueue, &msg, &xHigherPriorityTaskWoken);
  portYIELD_FROM_ISR(xHigherPriorityTaskWoken);
  //if (xHigherPriorityTaskWoken) {
  //  taskYIELD_FROM_ISR ();
  //}
}