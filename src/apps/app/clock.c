#include <stdint.h>
#include <stddef.h>
#include "clock.h"
#include "rtc.h"
#include "app.h"
#include "sys.h"
#include "utils.h"
#include "ble_cmd.h"
#include "lvgl.h"

static const app_spec_t clock_spec;

clock_app_t clock_app = {
    .app = {.spec = &clock_spec }
};

static inline clock_app_t *_from_app(app_t *app) {
    return container_of(app, clock_app_t, app);
}


lv_obj_t *screen_clock_create(clock_app_t *ht, lv_obj_t * parent) {
    UTCTimeStruct time_tmp;

    lv_obj_t *scr = lv_obj_create(parent);

    lv_obj_clear_flag(scr, LV_OBJ_FLAG_CLICKABLE);
    lv_obj_clear_flag(scr, LV_OBJ_FLAG_SCROLL_ELASTIC);
    lv_obj_remove_style_all(scr);                            /*Make it transparent*/
    lv_obj_set_size(scr, lv_pct(100), lv_pct(100));
    //lv_obj_set_scroll_snap_y(scr, LV_SCROLL_SNAP_CENTER);    /*Snap the children to the center*/

    //lv_obj_t *scr = lv_scr_act();

    lv_obj_set_style_bg_color(scr, lv_color_hex(0x000000), 0);

    get_UTC_time(&time_tmp);

    lv_obj_t * lv_timeh = lv_label_create(scr);    
    lv_obj_set_style_text_font(lv_timeh, &lv_font_clock_90, 0);
    lv_label_set_text_fmt(lv_timeh, "%02i", time_tmp.hour);
    lv_obj_set_style_text_align(lv_timeh, LV_TEXT_ALIGN_CENTER, 0);
    lv_obj_set_style_text_color(lv_timeh, lv_color_make(0xff, 0xff, 0xff), 0);
    lv_obj_align(lv_timeh, LV_ALIGN_CENTER, -55, -40);

    ht->lv_timeh = lv_timeh;

    lv_obj_t * lv_timem = lv_label_create(scr);    
    lv_obj_set_style_text_font(lv_timem, &lv_font_clock_90, 0);
    lv_label_set_text_fmt(lv_timem, "%02i", time_tmp.minutes);
    lv_obj_set_style_text_align(lv_timem, LV_TEXT_ALIGN_CENTER, 0);
    lv_obj_set_style_text_color(lv_timem, lv_color_make(0x00, 0xff, 0x00), 0);
    lv_obj_align(lv_timem, LV_ALIGN_CENTER, 55, -40);

    ht->lv_timem = lv_timem;

    lv_obj_t * lv_times = lv_label_create(scr);    
    lv_obj_set_style_text_font(lv_times, &lv_font_clock_90, 0);
    lv_label_set_text_static(lv_times, ":");
    lv_obj_set_style_text_align(lv_times, LV_TEXT_ALIGN_CENTER, 0);
    lv_obj_set_style_text_color(lv_times, lv_color_make(0xff, 0xff, 0x00), 0);
    lv_obj_align(lv_times, LV_ALIGN_CENTER, 0, -40);

    ht->lv_times = lv_times;

    lv_obj_t * lv_date = lv_label_create(scr);    
    //lv_obj_set_style_text_font(lv_date, &lv_font_clock_42, 0);
    lv_label_set_recolor(lv_date, true);
    lv_label_set_text_fmt(lv_date, "#00ff00 %s# %02i %s", get_days(time_tmp.week), time_tmp.day, get_months(time_tmp.month));
    lv_obj_set_style_text_align(lv_date, LV_TEXT_ALIGN_CENTER, 0);
    lv_obj_set_style_text_color(lv_date, lv_color_make(0xff, 0xff, 0xff), 0);
    lv_obj_align(lv_date, LV_ALIGN_CENTER, 0, 20);

    ht->lv_date = lv_date;

    lv_obj_t * lv_ble = lv_label_create(scr);    
    lv_obj_align(lv_ble, LV_ALIGN_TOP_LEFT, 0, 0);
    lv_obj_set_style_text_font(lv_ble, &lv_font_sys_20, 0);
    lv_label_set_text(lv_ble, "\xEE\xA4\x83");
    if ( pinetimecos.bluetoothState == StatusOFF ) {
        lv_obj_set_style_text_color(lv_ble, lv_color_hex(0x909090), 0);
    } else {
        lv_obj_set_style_text_color(lv_ble, lv_color_hex(0x0000ff), 0);
    }

    ht->lv_ble = lv_ble;

    lv_obj_t * lv_power = lv_label_create(scr);
    lv_obj_set_style_text_color(lv_power, lv_color_hex(0xffffff), 0);
    lv_obj_align(lv_power, LV_ALIGN_TOP_RIGHT, 0, 0);
    lv_obj_set_style_text_font(lv_power, &lv_font_sys_20, 0);
    if ( pinetimecos.chargingState == StatusOFF ) {
        lv_label_set_text(lv_power, "\xEE\xA4\x87");
    } else {
        lv_label_set_text(lv_power, "\xEE\xA4\x85 \xEE\xA4\x87");
    }

    ht->lv_power = lv_power;


    /*lv_obj_t * lv_time_sec = lv_label_create(scr);
    lv_label_set_long_mode(lv_time_sec, LV_LABEL_LONG_SCROLL_CIRCULAR);     
    lv_label_set_text(lv_time_sec, "It is a circularly scrolling text. is big....");
    lv_obj_set_width(lv_time_sec, 220);
    //lv_obj_set_style_text_align(lv_time_sec, LV_TEXT_ALIGN_CENTER, 0);
    lv_obj_set_style_text_color(lv_time_sec, lv_color_make(0xaf, 0xaf, 0xaf), 0);
    lv_obj_align(lv_time_sec, LV_ALIGN_CENTER, 0, 75);

    ht->lv_time_sec = lv_time_sec;*/

    ht->time_old = time_tmp;


    lv_obj_t * lv_debug = lv_label_create(scr);
    lv_obj_set_style_text_color(lv_debug, lv_color_make(0xff, 0xf, 0xff), 0);
    lv_obj_align(lv_debug, LV_ALIGN_BOTTOM_LEFT, 0, -25);
    lv_obj_set_width(lv_debug, 240);
    lv_label_set_long_mode(lv_debug, LV_LABEL_LONG_SCROLL_CIRCULAR);
    if ( pinetimecosBLE.weather.hasData )
        lv_label_set_text_fmt(lv_debug, "%s - %s, %i°C ", pinetimecosBLE.weather.location, pinetimecosBLE.weather.currentCondition, pinetimecosBLE.weather.currentTemp);
    else 
        lv_label_set_text(lv_debug, "");

    ht->lv_demo = lv_debug;

    return scr;
}

int clock_init(app_t *app, lv_obj_t * parent) {
    clock_app_t *htapp = _from_app(app);
    htapp->screen = screen_clock_create(htapp, parent);
    return 0;
}

int clock_update(app_t *app) {
    UTCTimeStruct time_tmp;

    clock_app_t *ht = _from_app(app);
    get_UTC_time(&time_tmp);

    if (time_tmp.hour != ht->time_old.hour)
        lv_label_set_text_fmt(ht->lv_timeh, "%02i", time_tmp.hour);

    if (time_tmp.minutes != ht->time_old.minutes)
        lv_label_set_text_fmt(ht->lv_timem, "%02i", time_tmp.minutes);

    if ( time_tmp.seconds % 2 == 0 ) {
      lv_label_set_text_static(ht->lv_times,  ":");
    } else {
      lv_label_set_text_static(ht->lv_times,  " ");
    }

    //lv_label_set_text_fmt(ht->lv_time_sec, "%02i", time_tmp.seconds);

    if (time_tmp.day != ht->time_old.day)
        lv_label_set_text_fmt(ht->lv_date, "#00ff00 %s# %02i %s", get_days(time_tmp.week), time_tmp.day, get_months(time_tmp.month));

    if ( pinetimecos.bluetoothState == StatusOFF ) {
        lv_obj_set_style_text_color(ht->lv_ble, lv_color_hex(0x404040), 0);
    } else {
        lv_obj_set_style_text_color(ht->lv_ble, lv_color_hex(0x0000ff), 0);
    }

    if ( pinetimecos.chargingState == StatusOFF ) {
        lv_label_set_text(ht->lv_power, "\xEE\xA4\x87");
    } else {
        lv_label_set_text(ht->lv_power, "\xEE\xA4\x85 \xEE\xA4\x87");
    }

    if ( pinetimecosBLE.weather.newData ) {
        lv_label_set_text_fmt(ht->lv_demo, "%s - %s, %i°C ", pinetimecosBLE.weather.location, pinetimecosBLE.weather.currentCondition, pinetimecosBLE.weather.currentTemp);
        pinetimecosBLE.weather.newData = false;
    }

    ht->time_old = time_tmp;

    return 0;
}

int clock_close(app_t *app) {
    clock_app_t *ht = _from_app(app);
    lv_obj_clean(ht->screen);
    lv_obj_del(ht->screen);
    ht->screen = NULL;
    return 0;
}

static const app_spec_t clock_spec = {
    .name = "clock",
    .updateInterval = 250,
    .init = clock_init,
    .update = clock_update,
    .close = clock_close,
};