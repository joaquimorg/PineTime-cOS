#ifndef ST7789_H
#define ST7789_H

#include <stdbool.h>
#include <stdint.h>

#define RGB2COLOR(r, g, b) ((((r >> 3) << 11) | ((g >> 2) << 5) | (b >> 3)))

void st7789_send(uint8_t cmd, const uint8_t *data, unsigned len);
void st7789_init(void);
void st7789_set_window(uint32_t x, uint32_t y, uint32_t w, uint32_t h);
void st7789_fill(uint16_t x1, uint16_t y1, uint16_t x2, uint16_t y2, uint16_t color);


#endif /* ST7789_H */