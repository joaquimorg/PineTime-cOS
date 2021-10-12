#ifndef INFO_H
#define INFO_H

#include <stdbool.h>
#include <stdint.h>
#include "app.h"
#include "lvgl.h"

typedef struct _info_app {
    app_t app;
    lv_obj_t *screen;
    lv_obj_t *lv_demo;
} info_app_t;


extern info_app_t info_app;
#define APP_INFO (&info_app.app)

#endif /* INFO_H */