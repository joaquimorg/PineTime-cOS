#include "nordic_common.h"
#include "nrf_delay.h"

/* FreeRTOS related */
#include "FreeRTOS.h"
#include "task.h"
#include "semphr.h"

#include "app.h"
#include "lvgl.h"
#include "rtc.h"


#define APP_TASK_DELAY        50

lv_obj_t *time_text;
UTCTimeStruct time_tmp;
lv_timer_t * timer;

void update_time(lv_timer_t * timer) {
    get_UTC_time(&time_tmp);
    lv_label_set_text_fmt(time_text, "%02i:%02i:%02i", time_tmp.hour, time_tmp.minutes, time_tmp.seconds);
}

void lv_example(void)
{

    time_text = lv_label_create(lv_scr_act());
    
    get_UTC_time(&time_tmp);

    lv_obj_set_style_text_font(time_text, &lv_font_montserrat_32, 0);
    
    lv_label_set_text_fmt(time_text, "%02i:%02i:%02i", time_tmp.hour, time_tmp.minutes, time_tmp.seconds);
    lv_obj_set_style_text_align(time_text, LV_TEXT_ALIGN_CENTER, 0);
    lv_obj_align(time_text, LV_ALIGN_CENTER, 0, 0);

    timer = lv_timer_create(update_time, 1000, NULL);
    lv_timer_ready(timer);
}



void main_app(void* pvParameter) {
    UNUSED_PARAMETER(pvParameter);

    enum appMessages msg;
    appMsgQueue = xQueueCreate(QUEUESIZE, ITEMSIZE);

    lv_example();

    /*lv_obj_t* spinner = lv_spinner_create(lv_scr_act(), 1000, 60);
    lv_obj_set_size(spinner, 100, 100);
    lv_obj_center(spinner);*/

    while (true) {

        if (xQueueReceive(appMsgQueue, &msg, ( TickType_t ) 10)) {
            switch (msg)
            {
                case GoToSleep:
                    /* code */
                    break;
                
                default:
                    break;
            }
        }

        vTaskDelay(APP_TASK_DELAY);
        /* Tasks must be implemented to never return... */
    }

}