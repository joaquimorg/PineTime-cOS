#ifndef APP_H
#define APP_H

#include <stdbool.h>
#include <stdint.h>
#include "FreeRTOS.h"
#include "queue.h"

enum appMessages {
     GoToSleep              = 0x01,
     GoToRunning            = 0x02, 
     UpdateBleConnection    = 0x03,
     UpdateBatteryLevel     = 0x04,
     ButtonPushed           = 0x05
};

QueueHandle_t appMsgQueue;
#define QUEUESIZE   10
#define ITEMSIZE    1


void main_app(void* pvParameter);


#endif // APP_H