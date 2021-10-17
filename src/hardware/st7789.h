#ifndef ST7789_H
#define ST7789_H

#include <stdbool.h>
#include <stdint.h>

#include "lvgl.h"

#define RGB2COLOR(r, g, b) ((((r >> 3) << 11) | ((g >> 2) << 5) | (b >> 3)))

void drawBitmap (uint16_t x1, uint16_t y1, uint16_t x2, uint16_t y2, uint8_t* bitmap);
void drawSquare(uint16_t x1, uint16_t y1, uint16_t x2, uint16_t y2, uint16_t color);

void st7789_vertical_scroll_definition(uint16_t topFixedLines, uint16_t scrollLines, uint16_t bottomFixedLines, uint16_t line);

//void st7789_send(uint8_t cmd, const uint8_t *data, unsigned len);
//void st7789_set_window(uint32_t x, uint32_t y, uint32_t w, uint32_t h);
//void st7789_fill(uint16_t x1, uint16_t y1, uint16_t x2, uint16_t y2, uint16_t color);
//void st7789_vertical_scroll_start_address(uint16_t line);
//void st7789_vertical_scroll_definition(uint16_t topFixedLines, uint16_t scrollLines, uint16_t bottomFixedLines);
//void st7789_display_off(void);
//void st7789_display_on(void);
void st7789_sleep_out(void);
void st7789_sleep_in(void);

void st7789_init(void);

void st7789_sleep(void);
void st7789_wake_up(void);


#endif /* ST7789_H */