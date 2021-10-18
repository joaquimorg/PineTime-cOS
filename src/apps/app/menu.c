

#include <stdint.h>
#include <stddef.h>
#include "menu.h"
#include "app.h"
#include "sys.h"
#include "utils.h"
#include "lvgl.h"

static const app_spec_t menu_spec;

menu_app_t menu_app = {
    .app = {.spec = &menu_spec }
};

static inline menu_app_t *_from_app(app_t *app) {
    return container_of(app, menu_app_t, app);
}


lv_obj_t *screen_menu_create(menu_app_t *ht, lv_obj_t * parent) {

    lv_obj_t *scr = lv_obj_create(parent);

    lv_obj_clear_flag(scr, LV_OBJ_FLAG_CLICKABLE);
    lv_obj_clear_flag(scr, LV_OBJ_FLAG_SCROLL_ELASTIC);
    lv_obj_remove_style_all(scr);                            /*Make it transparent*/
    lv_obj_set_size(scr, lv_pct(100), lv_pct(100));
    lv_obj_set_scroll_snap_y(scr, LV_SCROLL_SNAP_CENTER);    /*Snap the children to the center*/

    //lv_obj_t *scr = parent;

    lv_obj_set_style_bg_color(scr, lv_color_hex(0x000000), 0);

    LV_IMG_DECLARE(msg);
    lv_obj_t * lv_img = lv_img_create(scr);
    lv_img_set_src(lv_img, &msg);
    lv_obj_align(lv_img, LV_ALIGN_CENTER, 0, -20);
    ht->lv_img = lv_img;

    lv_obj_t * lv_title = lv_label_create(scr);    
    lv_label_set_text(lv_title, "Messages");
    lv_obj_set_style_text_align(lv_title, LV_TEXT_ALIGN_CENTER, 0);
    lv_obj_set_style_text_color(lv_title, lv_color_hex(0x00ff1f), 0);
    lv_obj_align(lv_title, LV_ALIGN_CENTER, 0, 60);

    ht->lv_title = lv_title;

    return scr;
}

int menu_init(app_t *app, lv_obj_t * parent) {
    menu_app_t *htapp = _from_app(app);
    htapp->screen = screen_menu_create(htapp, parent);
    return 0;
}

int menu_update(app_t *app) {

    //menu_app_t *ht = _from_app(app);    

    return 0;
}

int menu_close(app_t *app) {
    menu_app_t *ht = _from_app(app);
    lv_obj_clean(ht->screen);
    lv_obj_del(ht->screen);    
    ht->screen = NULL;
    return 0;
}

static const app_spec_t menu_spec = {
    .name = "menu",
    .updateInterval = 1000,
    .init = menu_init,
    .update = menu_update,
    .close = menu_close,
};