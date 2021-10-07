#ifndef _SYS_H
#define _SYS_H

#include <stdint.h>
#include "nordic_common.h"
#include "FreeRTOS.h"

enum State {
     Sleep              = 0x01,
     Running            = 0x02
};

/* Main struct to for global state */
struct pinetimecOS
{

    BaseType_t appTask;

    uint8_t backlightValue;
    enum State state;
};

struct pinetimecOS pinetimecos;


void sys_init(void);


#endif // _SYS_H