#ifndef DEBUG_H
#define DEBUG_H

#include <stdbool.h>
#include <stdint.h>
#include "app.h"
#include "lvgl.h"

typedef struct _debug_app {
    app_t app;
    lv_obj_t *screen;
    lv_obj_t *lv_demo;
    lv_obj_t *lv_table;
} debug_app_t;


extern debug_app_t debug_app;
#define APP_DEBUG (&debug_app.app)

#endif /* DEBUG_H */