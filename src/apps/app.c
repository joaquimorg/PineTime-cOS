#include "nordic_common.h"
#include "nrf_delay.h"

/* FreeRTOS related */
#include "FreeRTOS.h"
#include "task.h"
#include "semphr.h"

#include "app.h"
#include "lvgl.h"


#define APP_TASK_DELAY        50


void lv_example(void)
{
    lv_obj_t * label1 = lv_label_create(lv_scr_act());
    lv_label_set_text(label1, "PineTime-cOS");
    lv_obj_set_style_text_font(label1, &lv_font_montserrat_32, 0);
    lv_obj_set_style_text_align(label1, LV_TEXT_ALIGN_CENTER, 0);
    lv_obj_align(label1, LV_ALIGN_TOP_MID, 0, 0);

    lv_obj_t * spinner = lv_spinner_create(lv_scr_act(), 1000, 80);
    lv_obj_set_size(spinner, 150, 150);
    lv_obj_center(spinner);

    lv_obj_t * btn2 = lv_btn_create(lv_scr_act());    
    lv_obj_align(btn2, LV_ALIGN_CENTER, 0, 40);
    lv_obj_add_flag(btn2, LV_OBJ_FLAG_CHECKABLE);
    lv_obj_set_height(btn2, LV_SIZE_CONTENT);
    label1 = lv_label_create(btn2);
    lv_label_set_text(label1, "Toggle");
    lv_obj_center(label1);
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