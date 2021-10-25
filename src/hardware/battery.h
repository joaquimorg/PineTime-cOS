#ifndef BATTERY_CONFIG_H
#define BATTERY_CONFIG_H

#include "stdint.h"
#include "pinetime_board.h"
#include "nrf_gpio.h"

void battery_init(void);
void battery_read(void);
char * battery_get_icon(void);


#endif //BATTERY_CONFIG_H