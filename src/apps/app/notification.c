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

    /*static lv_point_t line_points[] = { {5, 236}, {234, 236} };

    lv_obj_t * line1;
    line1 = lv_line_create(ht->screen);
    lv_line_set_points(line1, line_points, 2);
    lv_obj_set_style_line_width(line1, 4, 0);
    lv_obj_set_style_line_color(line1, lv_color_hex(0x555555), 0);
    lv_obj_set_style_line_rounded(line1, true, 0);
    
    lv_obj_center(line1);*/
    
}

static void anim_x_cb(void * var, int32_t v) {
    lv_obj_set_x(var, v);
}

static int show_not(app_t *app);

static void anim_label_n(lv_obj_t *obj, bool reverse) {

    lv_anim_t a;    
    lv_anim_init(&a);

    lv_anim_set_var(&a, obj);
    if (reverse) {
        lv_anim_set_values(&a, 240, 0);
        lv_anim_set_path_cb(&a, lv_anim_path_ease_in);
    } else {
        lv_anim_set_values(&a, -240, 0);
        lv_anim_set_path_cb(&a, lv_anim_path_ease_out);
    }
    lv_anim_set_time(&a, 250);
    lv_anim_set_exec_cb(&a, anim_x_cb);
    lv_anim_start(&a);
}

static void anim_end_cb(lv_anim_t * a) {
    notification_app_t *ht = _from_app((app_t*)(a->user_data));    
    show_not((app_t*)(a->user_data));
    //lv_obj_set_x(ht->lv_not, 0);
    //lv_obj_fade_in(ht->lv_not, 200, 0);
    anim_label_n(ht->lv_not, ht->reverse);
}

static void anim_label(app_t *app, lv_obj_t *obj, bool cb, bool reverse) {

    lv_anim_t a;    
    lv_anim_init(&a);

    a.user_data = app;
    
    lv_anim_set_var(&a, obj);
    if (reverse) {
        lv_anim_set_values(&a, 0, 240);
        lv_anim_set_path_cb(&a, lv_anim_path_ease_out);
    } else {
        lv_anim_set_values(&a, 0, -240);
        lv_anim_set_path_cb(&a, lv_anim_path_ease_in);
    }
    lv_anim_set_time(&a, 250);
    lv_anim_set_exec_cb(&a, anim_x_cb);
    if (cb) {
        lv_anim_set_ready_cb(&a, anim_end_cb);
    }
    lv_anim_start(&a);
    //lv_obj_fade_out(obj, 200, 0);
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
    ht->lv_app = lv_app;
    lv_label_set_text_static(ht->lv_app, "No new\nnotifications");
    lv_obj_set_style_text_color(ht->lv_app, lv_color_hex(0xffff00), 0);
    lv_obj_align(ht->lv_app, LV_ALIGN_CENTER, 0, 0);
    lv_obj_center(ht->lv_app);
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
                ht->reverse = false;
                anim_label(app, ht->lv_not, true, true);
                //show_not(app);
            }
            break;
        case DirLeft:
            if (ht->current_not < pinetimecosBLE.notificationCount) {
                ht->current_not++;
                ht->reverse = true;
                anim_label(app, ht->lv_not, true, false);
                //show_not(app);
                //anim_label(app, ht->lv_not, false);
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