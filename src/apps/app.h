#ifndef APP_H
#define APP_H

#include <stdbool.h>
#include <stdint.h>
#include "FreeRTOS.h"
#include "queue.h"
#include "lvgl.h"


typedef struct _app app_t;
typedef struct app_spec {
     const char *name;
     uint32_t updateInterval;
     int (*init)(app_t *app, lv_obj_t * parent);
     int (*update)(app_t *app);
     int (*close)(app_t *app);
} app_spec_t;

struct _app {
    const app_spec_t *spec;
    bool dirty;
};

enum apps {
     Clock     = 0x01,
     Info      = 0x02,
     Info2     = 0x03,
};

enum appMessages {
     UpdateBleConnection    = 0x01,
     UpdateBatteryLevel     = 0x02,
     Timeout                = 0x03,
     ButtonPushed           = 0x04,
     WakeUp                 = 0x05,
     Charging               = 0x06,
     Gesture                = 0x07,
};

enum appGestures {
     DIR_NONE       = 0x00,
     DIR_LEFT       = 0x01,     
     DIR_RIGHT      = 0x02,
     DIR_TOP        = 0x03,
     DIR_BOTTOM     = 0x04,
};

enum RefreshDirections {
     None      = 0x00,
     Up        = 0x01,
     Down      = 0x02,
     Left      = 0x03,
     Right     = 0x04,
};


struct pinetimecOSApp {
     
     enum appGestures gestureDir;
     enum RefreshDirections refreshDirection;

     app_t *runningApp;
     enum apps returnApp;
     enum appGestures returnDir;
     enum RefreshDirections returnDirection;
     enum apps activeApp;
};

struct pinetimecOSApp pinetimecosapp;

QueueHandle_t appMsgQueue;
#define QUEUESIZE   10
#define ITEMSIZE    1

void main_app(void* pvParameter);
void app_push_message(enum appMessages msg);

#endif // APP_H