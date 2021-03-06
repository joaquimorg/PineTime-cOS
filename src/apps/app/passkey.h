#ifndef PASSKEY_H
#define PASSKEY_H

#include <stdbool.h>
#include <stdint.h>
#include "app.h"
#include "lvgl.h"

typedef struct _passkey_app {
    app_t app;
    lv_obj_t *screen;
} passkey_app_t;


extern passkey_app_t passkey_app;
#define APP_PASSKEY (&passkey_app.app)

#endif /* PASSKEY_H */