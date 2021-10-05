
#ifndef CST816_H
#define CST816_H

#include <stdbool.h>
#include <stdint.h>

#define TOUCH_NO_GESTURE 0x00
#define TOUCH_SLIDE_DOWN 0x01
#define TOUCH_SLIDE_UP 0x02
#define TOUCH_SLIDE_LEFT 0x03
#define TOUCH_SLIDE_RIGHT 0x04
#define TOUCH_SINGLE_CLICK 0x05
#define TOUCH_DOUBLE_CLICK 0x0B
#define TOUCH_LONG_PRESS 0x0C

struct touch_data_struct {
  uint8_t gesture;
  uint8_t touchpoints;
  uint8_t event;
  uint8_t xpos;
  uint8_t ypos;
  uint8_t last_xpos;
  uint8_t last_ypos;
  uint8_t version15;
  uint8_t versionInfo[3];
};

struct touch_data_struct tsData;

void cst816Init(void);

void cst816Get(void);

#endif // CST816_H