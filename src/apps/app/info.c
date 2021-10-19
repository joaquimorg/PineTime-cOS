#include <stdint.h>
#include <stddef.h>
#include "info.h"
#include "app.h"
#include "sys.h"
#include "utils.h"
#include "lvgl.h"


static const app_spec_t info_spec;

info_app_t info_app = {
    .app = {.spec = &info_spec }
};

static inline info_app_t *_from_app(app_t *app) {
    return container_of(app, info_app_t, app);
}

static void event_handler(lv_event_t * e)
{
 
    lv_event_code_t code = lv_event_get_code(e);

    if(code == LV_EVENT_CLICKED) {
        NVIC_SystemReset();
    }
    
}

lv_obj_t *screen_info_create(info_app_t *ht, lv_obj_t * parent) {

    lv_obj_t *scr = lv_obj_create(parent);

    lv_obj_clear_flag(scr, LV_OBJ_FLAG_CLICKABLE);
    lv_obj_clear_flag(scr, LV_OBJ_FLAG_SCROLL_ELASTIC);
    lv_obj_remove_style_all(scr);                            /*Make it transparent*/
    lv_obj_set_size(scr, lv_pct(100), lv_pct(100));
    //lv_obj_set_scroll_snap_y(scr, LV_SCROLL_SNAP_CENTER);    /*Snap the children to the center*/

    //lv_obj_t *scr = lv_scr_act();

    lv_obj_set_style_bg_color(scr, lv_color_hex(0x900000), 0);


    lv_obj_t * lv_demo = lv_label_create(scr);    
    lv_label_set_text_fmt(lv_demo, "Info\n\nBattery status\n%1i.%02i volts %d%%", pinetimecos.batteryVoltage / 1000, pinetimecos.batteryVoltage % 1000 / 10, pinetimecos.batteryPercentRemaining);
    lv_obj_set_style_text_align(lv_demo, LV_TEXT_ALIGN_CENTER, 0);
    lv_obj_set_style_text_color(lv_demo, lv_color_hex(0xffffff), 0);
    lv_obj_align(lv_demo, LV_ALIGN_CENTER, 0, -40);

    ht->lv_demo = lv_demo;


    lv_obj_t * label;

    lv_obj_t * btn1 = lv_btn_create(scr);
    lv_obj_add_event_cb(btn1, event_handler, LV_EVENT_ALL, NULL);
    lv_obj_align(btn1, LV_ALIGN_CENTER, 0, 60);

    label = lv_label_create(btn1);
    lv_label_set_text(label, "Demo");
    lv_obj_center(label);

    return scr;
}

int info_init(app_t *app, lv_obj_t * parent) {
    info_app_t *htapp = _from_app(app);
    htapp->screen = screen_info_create(htapp, parent);
    return 0;
}

int info_update(app_t *app) {

    info_app_t *ht = _from_app(app);

    lv_label_set_text_fmt(ht->lv_demo, "Info\n\nBattery status\n%1i.%02i volts %d%%", pinetimecos.batteryVoltage / 1000, pinetimecos.batteryVoltage % 1000 / 10, pinetimecos.batteryPercentRemaining);

    return 0;
}

int info_close(app_t *app) {
    info_app_t *ht = _from_app(app);
    lv_obj_clean(ht->screen);
    lv_obj_del(ht->screen);    
    ht->screen = NULL;
    return 0;
}

static const app_spec_t info_spec = {
    .name = "info",
    .updateInterval = 1000,
    .init = info_init,
    .update = info_update,
    .close = info_close,
};