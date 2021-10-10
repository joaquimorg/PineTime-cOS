#ifndef CLOCK_H
#define CLOCK_H

#include <stdbool.h>
#include <stdint.h>
#include "app.h"
#include "lvgl.h"

typedef struct _clock_app {
    app_t app;
    lv_obj_t *screen;
    lv_obj_t *lv_time;
    lv_obj_t *lv_time_sec;
    lv_obj_t *lv_date;
    lv_obj_t *lv_ble;
    lv_obj_t *lv_power;
    
} clock_app_t;


extern clock_app_t clock_app;
#define APP_CLOCK (&clock_app.app)

#endif /* CLOCK_H */