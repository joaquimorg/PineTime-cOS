#ifndef MENU_H
#define MENU_H

#include <stdbool.h>
#include <stdint.h>
#include "app.h"
#include "lvgl.h"

typedef struct _menu_app {
    app_t app;
    lv_obj_t *screen;
    lv_obj_t *lv_ble;
    lv_obj_t *lv_time;
    lv_obj_t *lv_power;
    lv_obj_t *lbl_dont_disturb;
    lv_obj_t *lbl_backlight;
} menu_app_t;


extern menu_app_t menu_app;
#define APP_MENU (&menu_app.app)

#endif /* MENU_H */