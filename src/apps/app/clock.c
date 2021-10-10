

#include <stdint.h>
#include <stddef.h>
#include "clock.h"
#include "rtc.h"
#include "app.h"
#include "sys.h"
#include "utils.h"
#include "lvgl.h"

static const app_spec_t clock_spec;

clock_app_t clock_app = {
    .app = {.spec = &clock_spec }
};

static inline clock_app_t *_from_app(app_t *app) {
    return container_of(app, clock_app_t, app);
}

lv_obj_t *screen_clock_create(clock_app_t *ht) {
    UTCTimeStruct time_tmp;

    //lv_obj_t *scr = lv_obj_create(lv_scr_act());
    lv_obj_t *scr = lv_scr_act();

    get_UTC_time(&time_tmp);

    lv_obj_t * lv_time = lv_label_create(scr);    
    lv_obj_set_style_text_font(lv_time, &lv_font_clock_90, 0);
    lv_label_set_text_fmt(lv_time, "%02i:%02i", time_tmp.hour, time_tmp.minutes);
    lv_obj_set_style_text_align(lv_time, LV_TEXT_ALIGN_CENTER, 0);
    lv_obj_set_style_text_color(lv_time, lv_color_make(0xff, 0xff, 0x00), 0);
    lv_obj_align(lv_time, LV_ALIGN_CENTER, -5, -40);

    ht->lv_time = lv_time;

    lv_obj_t * lv_time_sec = lv_label_create(scr);    
    lv_label_set_text_fmt(lv_time_sec, "%02i", time_tmp.seconds);
    lv_obj_set_style_text_align(lv_time_sec, LV_TEXT_ALIGN_CENTER, 0);
    lv_obj_set_style_text_color(lv_time_sec, lv_color_make(0xff, 0xff, 0xff), 0);
    lv_obj_align(lv_time_sec, LV_ALIGN_CENTER, 100, -65);

    ht->lv_time_sec = lv_time_sec;

    lv_obj_t * lv_date = lv_label_create(scr);    
    //lv_obj_set_style_text_font(lv_date, &lv_font_clock_42, 0);
    lv_label_set_text_fmt(lv_date, "%02i %s %04i", time_tmp.day, get_months_low(time_tmp.month), time_tmp.year);
    lv_obj_set_style_text_align(lv_date, LV_TEXT_ALIGN_CENTER, 0);
    lv_obj_set_style_text_color(lv_date, lv_color_make(0x3d, 0x5a, 0xfe), 0);
    lv_obj_align(lv_date, LV_ALIGN_CENTER, 0, 20);

    ht->lv_date = lv_date;

    lv_obj_t * lv_ble = lv_label_create(scr);
    lv_obj_set_style_text_color(lv_ble, lv_color_make(0x00, 0x00, 0xff), 0);
    lv_obj_align(lv_ble, LV_ALIGN_TOP_LEFT, 0, 0);
    lv_obj_set_style_text_font(lv_ble, &lv_font_sys_20, 0);
    lv_label_set_text(lv_ble, "");

    ht->lv_ble = lv_ble;

    lv_obj_t * lv_power = lv_label_create(scr);
    lv_obj_set_style_text_color(lv_power, lv_color_make(0x00, 0xff, 0x00), 0);
    lv_obj_align(lv_power, LV_ALIGN_TOP_RIGHT, 0, 0);
    lv_obj_set_style_text_font(lv_power, &lv_font_sys_20, 0);
    lv_label_set_text(lv_power, "\xEE\xA4\x87");

    ht->lv_power = lv_power;

    return scr;
}

int clock_init(app_t *app) {
    clock_app_t *htapp = _from_app(app);
    htapp->screen = screen_clock_create(htapp);
    return 0;
}

int clock_update(app_t *app) {
    UTCTimeStruct time_tmp;

    clock_app_t *ht = _from_app(app);
    get_UTC_time(&time_tmp);
    lv_label_set_text_fmt(ht->lv_time, "%02i:%02i", time_tmp.hour, time_tmp.minutes);
    lv_label_set_text_fmt(ht->lv_time_sec, "%02i", time_tmp.seconds);
    lv_label_set_text_fmt(ht->lv_date, "%02i %s %04i", time_tmp.day, get_months_low(time_tmp.month), time_tmp.year);

    if ( pinetimecos.bluetoothState == StatusOFF ) {
        lv_label_set_text(ht->lv_ble, "");
    } else {
        lv_label_set_text(ht->lv_ble, "\xEE\xA4\x83");
    }

    if ( pinetimecos.chargingState == StatusOFF ) {
        lv_label_set_text(ht->lv_power, "\xEE\xA4\x87");
    } else {
        lv_label_set_text(ht->lv_power, "\xEE\xA4\x85 \xEE\xA4\x87");
    }

    return 0;
}

int clock_close(app_t *app) {
    clock_app_t *ht = _from_app(app);
    lv_obj_del(ht->screen);
    ht->screen = NULL;
    return 0;
}

static const app_spec_t clock_spec = {
    .name = "clock",
    .init = clock_init,
    .update = clock_update,
    .close = clock_close,
};