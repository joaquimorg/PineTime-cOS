

#include <stdint.h>
#include <stddef.h>
#include "menu.h"
#include "app.h"
#include "sys.h"
#include "battery.h"
#include "backlight.h"
#include "rtc.h"
#include "utils.h"
#include "lvgl.h"

static const app_spec_t menu_spec;

menu_app_t menu_app = {
    .app = {.spec = &menu_spec }
};

static inline menu_app_t *_from_app(app_t *app) {
    return container_of(app, menu_app_t, app);
}


lv_obj_t *screen_create(menu_app_t *ht, lv_obj_t * parent) {

    lv_obj_t *scr = lv_obj_create(parent);

    lv_obj_clear_flag(scr, LV_OBJ_FLAG_CLICKABLE);
    lv_obj_clear_flag(scr, LV_OBJ_FLAG_SCROLL_ELASTIC);
    lv_obj_remove_style_all(scr);                            /*Make it transparent*/
    lv_obj_set_size(scr, lv_pct(100), lv_pct(100));
    //lv_obj_set_scroll_snap_y(scr, LV_SCROLL_SNAP_CENTER);    /*Snap the children to the center*/

    return scr;
}


static void btn_handler_settings(lv_event_t * e) {
    lv_event_code_t code = lv_event_get_code(e);

    if(code == LV_EVENT_CLICKED) {
        load_application(Debug, AnimDown);
    }
}

static void btn_handler_dont_disturb(lv_event_t * e) {
    
    lv_obj_t * obj = lv_event_get_target(e);

    if(lv_obj_has_state(obj, LV_STATE_CHECKED)) {
        pinetimecos.dontDisturb = false;
    } else {
        pinetimecos.dontDisturb = true;
    }
}

static void btn_handler_backlight(lv_event_t * e) {
    lv_event_code_t code = lv_event_get_code(e);

    if(code == LV_EVENT_CLICKED) {
        if ( pinetimecos.backlightLevel < 3 ) {
            pinetimecos.backlightLevel++;
        } else {
            pinetimecos.backlightLevel = 1;
        }
        set_backlight_level(pinetimecos.backlightLevel);
    }
}

static void menu_create(menu_app_t *ht) {

    UTCTimeStruct time_tmp;
    get_UTC_time(&time_tmp);

    lv_obj_t * lv_ble = lv_label_create(ht->screen);
    ht->lv_ble = lv_ble;
    lv_obj_align(ht->lv_ble, LV_ALIGN_TOP_RIGHT, -5, 5);
    lv_label_set_text(ht->lv_ble, "\xEE\xA4\x83");
    if ( pinetimecos.bluetoothState == StatusOFF ) {
        lv_obj_set_style_text_color(ht->lv_ble, lv_color_hex(0x606060), 0);
    } else {
        lv_obj_set_style_text_color(ht->lv_ble, lv_color_hex(0x000060), 0);
    }

    lv_obj_t * lv_power = lv_label_create(ht->screen);
    ht->lv_power = lv_power;
    lv_obj_set_style_text_color(ht->lv_power, lv_color_hex(0x606060), 0);
    lv_obj_align(ht->lv_power, LV_ALIGN_TOP_RIGHT, -34, 5);
    if ( pinetimecos.chargingState == StatusOFF ) {
        lv_label_set_text_fmt(ht->lv_power, "%s %i%%", battery_get_icon(), pinetimecos.batteryPercentRemaining == -1 ? 0 : pinetimecos.batteryPercentRemaining);
    } else {
        lv_label_set_text(ht->lv_power, "\xEE\xA4\x85 \xEE\xA4\xA0");
    }
    
    lv_obj_t * lv_time = lv_label_create(ht->screen);
    ht->lv_time = lv_time;
    lv_obj_set_style_text_color(ht->lv_time, lv_color_hex(0x606060), 0);
    lv_obj_align(ht->lv_time, LV_ALIGN_TOP_LEFT, 5, 5);
    lv_label_set_text_fmt(ht->lv_time, "%02d:%02d", time_tmp.hour, time_tmp.minutes);

    lv_obj_t * btn_dont_disturb = lv_btn_create(ht->screen);
    lv_obj_add_event_cb(btn_dont_disturb, btn_handler_dont_disturb, LV_EVENT_CLICKED, NULL);
    lv_obj_add_flag(btn_dont_disturb, LV_OBJ_FLAG_CHECKABLE);
    lv_obj_add_state(btn_dont_disturb, pinetimecos.dontDisturb ? LV_STATE_DEFAULT : LV_STATE_CHECKED);
    lv_obj_align(btn_dont_disturb, LV_ALIGN_CENTER, -55, -20);
    lv_obj_set_style_bg_color(btn_dont_disturb, lv_color_hex(0x006600), LV_STATE_CHECKED);

    lv_obj_t * lbl_dont_disturb = lv_label_create(btn_dont_disturb);
    ht->lbl_dont_disturb = lbl_dont_disturb;
    lv_obj_set_style_text_font(ht->lbl_dont_disturb, &lv_font_sys_48, 0);
    lv_label_set_text_static(ht->lbl_dont_disturb, pinetimecos.dontDisturb ? "\xEE\xA4\x8B" : "\xEE\xA4\x8C");
    lv_obj_center(ht->lbl_dont_disturb);    

    lv_obj_t * btn_backlight = lv_btn_create(ht->screen);
    lv_obj_add_event_cb(btn_backlight, btn_handler_backlight, LV_EVENT_CLICKED, NULL);    
    lv_obj_align(btn_backlight, LV_ALIGN_CENTER, 55, -20);

    lv_obj_t * lbl_backlight = lv_label_create(btn_backlight);
    ht->lbl_backlight = lbl_backlight;
    lv_obj_set_style_text_font(ht->lbl_backlight, &lv_font_sys_48, 0);
    lv_label_set_text_static(ht->lbl_backlight, get_backlight_icon(pinetimecos.backlightLevel));
    lv_obj_center(ht->lbl_backlight);

    lv_obj_t * btn_settings = lv_btn_create(ht->screen);
    lv_obj_add_event_cb(btn_settings, btn_handler_settings, LV_EVENT_CLICKED, NULL);
    lv_obj_align(btn_settings, LV_ALIGN_CENTER, 0, 65);

    lv_obj_t * btn_label = lv_label_create(btn_settings);
    lv_obj_set_style_text_font(btn_label, &lv_font_sys_48, 0);
    lv_label_set_text_static(btn_label, "\xEE\xA4\x82");
    lv_obj_center(btn_label);


}

int init(app_t *app, lv_obj_t * parent) {
    menu_app_t *htapp = _from_app(app);
    htapp->screen = screen_create(htapp, parent);
    menu_create(htapp);
    return 0;
}

int update(app_t *app) {

    UTCTimeStruct time_tmp;
    get_UTC_time(&time_tmp);

    menu_app_t *ht = _from_app(app);

    if ( pinetimecos.bluetoothState == StatusOFF ) {
        lv_obj_set_style_text_color(ht->lv_ble, lv_color_hex(0x404040), 0);
    } else {
        lv_obj_set_style_text_color(ht->lv_ble, lv_color_hex(0x0000ff), 0);
    }

    if ( pinetimecos.chargingState == StatusOFF ) {
        lv_label_set_text_fmt(ht->lv_power, "%s %i%%", battery_get_icon(), pinetimecos.batteryPercentRemaining == -1 ? 0 : pinetimecos.batteryPercentRemaining);
    } else {
        lv_label_set_text(ht->lv_power, "\xEE\xA4\x85 \xEE\xA4\xA0");
    }

    lv_label_set_text_fmt(ht->lv_time, "%02d:%02d", time_tmp.hour, time_tmp.minutes);

    lv_label_set_text_static(ht->lbl_dont_disturb, pinetimecos.dontDisturb ? "\xEE\xA4\x8B" : "\xEE\xA4\x8C");

    lv_label_set_text_static(ht->lbl_backlight, get_backlight_icon(pinetimecos.backlightLevel));

    return 0;
}

static int gesture(app_t *app, enum appGestures gesture) {
    return 0;
}

int close(app_t *app) {
    menu_app_t *ht = _from_app(app);
    lv_obj_clean(ht->screen);
    lv_obj_del(ht->screen);    
    ht->screen = NULL;
    return 0;
}

static const app_spec_t menu_spec = {
    .name = "menu",
    .updateInterval = 1000,
    .init = init,
    .update = update,
    .gesture = gesture,
    .close = close,
};