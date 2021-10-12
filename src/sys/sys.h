#ifndef _SYS_H
#define _SYS_H

#include <stdint.h>
#include "nordic_common.h"
#include "FreeRTOS.h"
#include "timers.h"

#include "app.h"

#define BUFFER_MAXCHARS 128

volatile uint8_t inputBuffer[BUFFER_MAXCHARS];

enum State {
     Sleep              = 0x01,
     Running            = 0x02
};

enum Status {
     StatusON           = 0x01,
     StatusOFF          = 0x02
};

enum RefreshDirections {
     None      = 0x00,
     Up        = 0x01,
     Down      = 0x02,
     Left      = 0x03,
     Right     = 0x04,
};

/* Main struct to for global state */
struct pinetimecOS {

     uint8_t backlightValue;
     enum State state;
     enum State lvglstate;
     enum Status bluetoothState;
     enum Status chargingState;
     enum Status powerState;
     enum appGestures gestureDir;
     enum RefreshDirections refreshDirection;
     uint16_t debug;
};

struct pinetimecOS pinetimecos;

TimerHandle_t idleTimer;
void reload_idle_timer(void);

void sys_init(void);


#endif // _SYS_H