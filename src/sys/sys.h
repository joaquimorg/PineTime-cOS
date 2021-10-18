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


/* Main struct to for global state */
struct pinetimecOS {

     uint8_t backlightValue;
     uint16_t batteryVoltage;
     int batteryPercentRemaining;
     enum Status chargingState;
     enum Status powerState;

     enum State state;
     enum State lvglstate;
     enum Status bluetoothState;

     uint16_t displayTimeout;
     uint16_t debug;
};

struct pinetimecOS pinetimecos;

TimerHandle_t idleTimer;
TimerHandle_t bleTimer;

void reload_idle_timer(void);

void sys_init(void);


#endif // _SYS_H