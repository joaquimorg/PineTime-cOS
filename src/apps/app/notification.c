#include <stdint.h>
#include <stddef.h>
#include "notification.h"
#include "app.h"
#include "sys.h"
#include "utils.h"
#include "ble_cmd.h"
#include "lvgl.h"


static const app_spec_t notification_spec;

notification_app_t notification_app = {
    .app = {.spec = &notification_spec }
};

static inline notification_app_t *_from_app(app_t *app) {
    return container_of(app, notification_app_t, app);
}


static void screen_create(notification_app_t *ht, lv_obj_t * parent) {
    lv_obj_t *scr = lv_obj_create(parent);

    lv_obj_clear_flag(scr, LV_OBJ_FLAG_CLICKABLE);
    lv_obj_clear_flag(scr, LV_OBJ_FLAG_SCROLL_ELASTIC);
    lv_obj_remove_style_all(scr);                            /*Make it transparent*/
    lv_obj_set_size(scr, lv_pct(100), lv_pct(100));

    ht->screen = scr;

    /*LV_IMG_DECLARE(msg);
    lv_obj_t * lv_img = lv_img_create(ht->screen);
    lv_img_set_src(lv_img, &msg);
    lv_obj_align(lv_img, LV_ALIGN_CENTER, 0, 0);*/
    
}

static void screen_notification_create(notification_app_t *ht) {

    notification_t notification;

    notification = pinetimecosBLE.notification[ht->current_not - 1];

    lv_obj_t * lv_count = lv_label_create(ht->screen);
    ht->lv_count = lv_count;
    lv_label_set_text_fmt(ht->lv_count, "%i / %i", ht->current_not, pinetimecosBLE.notificationCount);
    lv_obj_set_style_text_color(ht->lv_count, lv_color_hex(0x555555), 0);
    lv_obj_align(ht->lv_count, LV_ALIGN_TOP_RIGHT, 0, 0);

    lv_obj_t * lv_app = lv_label_create(ht->screen);
    ht->lv_app = lv_app;
    lv_label_set_text(ht->lv_app, notification.typeName);
    lv_obj_set_style_text_color(ht->lv_app, lv_color_hex(0x00ff00), 0);
    lv_obj_align(ht->lv_app, LV_ALIGN_TOP_LEFT, 0, 0);
    

    lv_obj_t * lv_not = lv_label_create(ht->screen);
    ht->lv_not = lv_not;
    lv_obj_align(ht->lv_not, LV_ALIGN_TOP_LEFT, 0, 40);
    lv_obj_set_width(ht->lv_not, 220);
    lv_label_set_long_mode(ht->lv_not, LV_LABEL_LONG_WRAP);
    lv_label_set_text_fmt(ht->lv_not, "%s\n%s", notification.subject, notification.body);

    pinetimecosBLE.newNotification = false;
    
}

static void no_notification_create(notification_app_t *ht) {
    lv_obj_t * lv_app = lv_label_create(ht->screen);    
    lv_label_set_text(lv_app, "No new notifications");
    lv_obj_set_style_text_color(lv_app, lv_color_hex(0xffff00), 0);
    //lv_obj_align(lv_app, LV_ALIGN_CENTER, 0, 0);
    lv_obj_center(lv_app);
}

static int init(app_t *app, lv_obj_t * parent) {
    notification_app_t *htapp = _from_app(app);
    screen_create(htapp, parent);
    
    if (pinetimecosBLE.notificationCount > 0) {
        htapp->current_not = pinetimecosBLE.notificationCount;
        screen_notification_create(htapp);
    } else {
        no_notification_create(htapp);
    }
    return 0;
}

static int show_not(app_t *app) {
    notification_t notification;
    notification_app_t *ht = _from_app(app);

    notification = pinetimecosBLE.notification[ht->current_not - 1];
    lv_label_set_text_fmt(ht->lv_count, "%i / %i", ht->current_not, pinetimecosBLE.notificationCount);
    lv_label_set_text(ht->lv_app, notification.typeName);
    lv_label_set_text_fmt(ht->lv_not, "%s\n%s", notification.subject, notification.body);

    return 0;
}

static int update(app_t *app) {
    
    notification_app_t *ht = _from_app(app);

    if ( pinetimecosBLE.newNotification ) {
        ht->current_not = pinetimecosBLE.notificationCount;
        pinetimecosBLE.newNotification = false;
        show_not(app);    
    }

    return 0;
}

static int gesture(app_t *app, enum appGestures gesture) {
    notification_app_t *ht = _from_app(app);

    if (pinetimecosBLE.notificationCount == 0) return 0;
    switch (gesture) {
        case DirBottom:
            break;
        case DirTop:
            break;
        case DirRight:
            if (ht->current_not > 1) {
                ht->current_not--;
                show_not(app);
            }
            break;
        case DirLeft:
            if (ht->current_not < pinetimecosBLE.notificationCount) {
                ht->current_not++;
                show_not(app);
            }
            break;
        default:
            break;
    }
    return 0;
}

static int close(app_t *app) {
    notification_app_t *ht = _from_app(app);
    lv_obj_clean(ht->screen);
    lv_obj_del(ht->screen);    
    ht->screen = NULL;
    return 0;
}

static const app_spec_t notification_spec = {
    .name = "notification",
    .updateInterval = 1000,
    .init = init,
    .update = update,
    .gesture = gesture,
    .close = close,
};