#ifndef BACKLIGHT_CONFIG_H
#define BACKLIGHT_CONFIG_H

#include "stdint.h"

#include "pinetime_board.h"

#include "nrf_gpio.h"

void backlight_init(void);
void set_backlight_level(uint8_t level);


#endif //BACKLIGHT_CONFIG_H