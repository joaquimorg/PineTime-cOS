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
     int (*init)(app_t *app);
     int (*update)(app_t *app);
     int (*close)(app_t *app);
} app_spec_t;

struct _app {
    const app_spec_t *spec;
    bool dirty;
};

enum apps {
     Clock    = 0x01,
};

enum appMessages {
     UpdateBleConnection    = 0x01,
     UpdateBatteryLevel     = 0x02,
     Timeout                = 0x03,
     ButtonPushed           = 0x04,
     TouchPushed            = 0x05,
     WakeUp                 = 0x06,
     Charging               = 0x07,
};

QueueHandle_t appMsgQueue;
#define QUEUESIZE   10
#define ITEMSIZE    1

app_t *active_app;

void main_app(void* pvParameter);
void app_push_message(enum appMessages msg);


int app_init(app_t *app);
int app_update(app_t *app);
int app_close(app_t *app);
void load_app(app_t *app);

#endif // APP_H