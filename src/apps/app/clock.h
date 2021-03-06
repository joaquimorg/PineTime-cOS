#ifndef CLOCK_H
#define CLOCK_H

#include <stdbool.h>
#include <stdint.h>
#include "app.h"
#include "rtc.h"
#include "lvgl.h"

typedef struct _clock_app {
    app_t app;
    lv_obj_t *screen;
    lv_obj_t *lv_timeh;
    lv_obj_t *lv_timem;
    lv_obj_t *lv_times;
    lv_obj_t *lv_date;
    lv_obj_t *lv_ble;
    lv_obj_t *lv_power;
    lv_obj_t *lv_demo;
    UTCTimeStruct time_old;
} clock_app_t;


extern clock_app_t clock_app;
#define APP_CLOCK (&clock_app.app)

#endif /* CLOCK_H */