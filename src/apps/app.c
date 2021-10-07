#include "nordic_common.h"
#include "nrf_delay.h"

/* FreeRTOS related */
#include "FreeRTOS.h"
#include "task.h"
#include "semphr.h"

#include "app.h"
#include "sys.h"
#include "lvgl.h"
#include "rtc.h"


#define APP_TASK_DELAY          10
#define APP_TASK_DELAY_SLEEP    100


static void anim_x_cb(void * var, int32_t v)
{
    lv_obj_set_x(var, v);
}
/*
static void anim_size_cb(void * var, int32_t v)
{
    lv_obj_set_size(var, v, v);
}
*/
/**
 * Create a playback animation
 */
void lv_example_anim_2(void)
{

    lv_obj_t * obj = lv_obj_create(lv_scr_act());
    lv_obj_set_size(obj, 40, 40);
    lv_obj_set_style_bg_color(obj, lv_color_make(0x00, 0xff, 0x00), 0);
    lv_obj_set_style_line_width(obj, 0, 0);
    lv_obj_set_style_radius(obj, LV_RADIUS_CIRCLE, 0);

    lv_obj_align(obj, LV_ALIGN_LEFT_MID, 0, 0);

    lv_anim_t a;
    lv_anim_init(&a);
    lv_anim_set_var(&a, obj);
    //lv_anim_set_values(&a, 10, 40);
    lv_anim_set_time(&a, 500);
    lv_anim_set_playback_delay(&a, 500);
    lv_anim_set_playback_time(&a, 500);
    lv_anim_set_repeat_delay(&a, 500);
    lv_anim_set_repeat_count(&a, LV_ANIM_REPEAT_INFINITE);
    lv_anim_set_path_cb(&a, lv_anim_path_ease_in_out);

    //lv_anim_set_exec_cb(&a, anim_size_cb);
    //lv_anim_start(&a);
    lv_anim_set_exec_cb(&a, anim_x_cb);
    lv_anim_set_values(&a, 0, 195);
    lv_anim_start(&a);
}

lv_obj_t *time_text;
UTCTimeStruct time_tmp;
lv_timer_t * timer;

void update_time(lv_timer_t * timer) {
    get_UTC_time(&time_tmp);
    lv_label_set_text_fmt(time_text, "%02i:%02i:%02i", time_tmp.hour, time_tmp.minutes, time_tmp.seconds);
}

LV_FONT_DECLARE(lv_font_clock_42)

static void drag_event_handler(lv_event_t * e)
{
    lv_obj_t * obj = lv_event_get_target(e);

    lv_indev_t * indev = lv_indev_get_act();
    lv_point_t vect;
    lv_indev_get_vect(indev, &vect);

    lv_coord_t x = lv_obj_get_x(obj) + vect.x;
    lv_coord_t y = lv_obj_get_y(obj) + vect.y;
    lv_obj_set_pos(obj, x, y);
}

void lv_example(void)
{

    lv_obj_t * obj;
    obj = lv_obj_create(lv_scr_act());
    lv_obj_set_size(obj, 120, 50);
    lv_obj_add_event_cb(obj, drag_event_handler, LV_EVENT_PRESSING, NULL);

    lv_obj_t * label = lv_label_create(obj);
    lv_label_set_text(label, "Drag me");
     lv_obj_set_style_text_color(label, lv_color_make(0x00, 0x00, 0x00), 0);
    lv_obj_center(label);


    lv_example_anim_2();

    time_text = lv_label_create(lv_scr_act());
    
    get_UTC_time(&time_tmp);

    lv_obj_set_style_text_font(time_text, &lv_font_clock_42, 0);
    
    lv_label_set_text_fmt(time_text, "%02i:%02i:%02i", time_tmp.hour, time_tmp.minutes, time_tmp.seconds);
    lv_obj_set_style_text_align(time_text, LV_TEXT_ALIGN_CENTER, 0);
    lv_obj_align(time_text, LV_ALIGN_CENTER, 0, 0);

    timer = lv_timer_create(update_time, 500, NULL);

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
        //vTaskDelay(APP_TASK_DELAY);
        if(pinetimecos.state == Running) {
            if (xQueueReceive(appMsgQueue, &msg, ( TickType_t ) APP_TASK_DELAY)) {
                switch (msg)
                {
                    case GoToSleep:
                        /* code */
                        break;
                    
                    default:
                        break;
                }
            }        
            lv_tick_inc(APP_TASK_DELAY);
        } else {
            vTaskDelay(APP_TASK_DELAY_SLEEP);
        }
        /* Tasks must be implemented to never return... */
    }

}