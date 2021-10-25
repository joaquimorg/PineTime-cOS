#ifndef MOTOR_H
#define MOTOR_H

#include "stdint.h"
#include "pinetime_board.h"
#include "nrf_gpio.h"
#include <FreeRTOS.h>
#include "timers.h"

void motor_stop(void);
void motor_start(uint8_t durationMs);
void motor_init(void);


#endif //MOTOR_H