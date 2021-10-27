#ifndef APP_H
#define APP_H

#include <stdbool.h>
#include <stdint.h>
#include "FreeRTOS.h"
#include "queue.h"
#include "lvgl.h"

enum apps {
     Debug          = 0x01,
     Clock          = 0x02,     
     Menu           = 0x03,
     Notification   = 0x04,
};

enum appMessages {
     UpdateBleConnection    = 0x01,
     UpdateBatteryLevel     = 0x02,
     Timeout                = 0x03,
     ButtonPushed           = 0x04,
     WakeUp                 = 0x05,
     Charging               = 0x06,
     Gesture                = 0x07,
     NewNotification        = 0x08,
};

enum appGestures {
     DirNone       = 0x00,
     DirLeft       = 0x01,     
     DirRight      = 0x02,
     DirTop        = 0x03,
     DirBottom     = 0x04,
     DirClick      = 0x05,
};

enum RefreshDirections {
     AnimNone      = 0x00,
     AnimUp        = 0x01,
     AnimDown      = 0x02,
     AnimLeft      = 0x03,
     AnimRight     = 0x04,
};


typedef struct _app app_t;
typedef struct app_spec {
     const char *name;
     uint32_t updateInterval;
     int (*init)(app_t *app, lv_obj_t * parent);
     int (*update)(app_t *app);
     int (*gesture)(app_t *app, enum appGestures gesture);
     int (*close)(app_t *app);
} app_spec_t;

struct _app {
    const app_spec_t *spec;
    bool dirty;
};

struct pinetimecOSApp {
     
     enum appGestures gestureDir;
     enum RefreshDirections refreshDirection;

     app_t *runningApp;
     enum apps returnApp;
     enum appGestures returnDir;
     enum RefreshDirections returnAnimation;
     enum apps activeApp;
};

struct pinetimecOSApp pinetimecosapp;

QueueHandle_t appMsgQueue;
#define QUEUESIZE   10
#define ITEMSIZE    1

void main_app(void* pvParameter);
void app_push_message(enum appMessages msg);
void load_application(enum apps app, enum RefreshDirections dir);

#endif // APP_H