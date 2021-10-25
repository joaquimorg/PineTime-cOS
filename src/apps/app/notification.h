#ifndef NOTIFICATION_H
#define NOTIFICATION_H

#include <stdbool.h>
#include <stdint.h>
#include "app.h"
#include "lvgl.h"

typedef struct _notification_app {
    app_t app;
    lv_obj_t *screen;
    lv_obj_t *lv_app;
    lv_obj_t *lv_count;
    lv_obj_t *lv_not;
    uint8_t current_not;
} notification_app_t;


extern notification_app_t notification_app;
#define APP_NOTIFICATION (&notification_app.app)

#endif /* NOTIFICATION_H */