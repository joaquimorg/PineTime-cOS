

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

lv_obj_t *screen_info_create(info_app_t *ht, lv_obj_t * parent) {

    lv_obj_t *scr = lv_obj_create(parent);

    lv_obj_clear_flag(scr, LV_OBJ_FLAG_CLICKABLE);
    lv_obj_clear_flag(scr, LV_OBJ_FLAG_SCROLL_ELASTIC);
    lv_obj_remove_style_all(scr);                            /*Make it transparent*/
    lv_obj_set_size(scr, lv_pct(100), lv_pct(100));
    lv_obj_set_scroll_snap_y(scr, LV_SCROLL_SNAP_CENTER);    /*Snap the children to the center*/

    //lv_obj_t *scr = lv_scr_act();

    lv_obj_set_style_bg_color(scr, lv_color_hex(0x020210), 0);

    lv_mem_monitor_t mon;
    lv_mem_monitor(&mon);
    int used_size = mon.total_size - mon.free_size;;
    int used_kb = used_size / 1024;
    int used_kb_tenth = (used_size - (used_kb * 1024)) / 102;

    lv_obj_t * lv_demo = lv_label_create(scr);    
    lv_label_set_text_fmt(lv_demo, "Info\n%s\n%d.%d kB used (%d %%)\n%d%% frag.", actual_reset_reason(), used_kb,  used_kb_tenth, mon.used_pct, mon.frag_pct);
    lv_obj_set_style_text_align(lv_demo, LV_TEXT_ALIGN_CENTER, 0);
    lv_obj_set_style_text_color(lv_demo, lv_color_hex(0xffffff), 0);
    lv_obj_align(lv_demo, LV_ALIGN_CENTER, 0, 0);

    ht->lv_demo = lv_demo;

    return scr;
}

int info_init(app_t *app, lv_obj_t * parent) {
    info_app_t *htapp = _from_app(app);
    htapp->screen = screen_info_create(htapp, parent);
    return 0;
}

int info_update(app_t *app) {

    //info_app_t *ht = _from_app(app);

    //lv_label_set_text_fmt(ht->lv_demo, "Info...%i", pinetimecos.debug);

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
    .init = info_init,
    .update = info_update,
    .close = info_close,
};