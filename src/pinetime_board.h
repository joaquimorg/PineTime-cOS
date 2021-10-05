#ifndef PINETIME_BOARD_H
#define PINETIME_BOARD_H

#include <stdbool.h>
#include <stdint.h>

// Pinetime IO
#define LCD_LIGHT_1 14
#define KEY_ACTION 13
#define KEY_ENABLE 15

#define LCD_CSN 25
#define LCD_DC 18
#define LCD_RST 26
#define SPI_SCK 2
#define SPI_MOSI 3
#define SPI_MISO 4

#define TWI_SCL 7
#define TWI_SDA 6

#define TP_TWI_ADDR 0x15
#define TP_IRQ 28
#define TP_RST 10


#endif // PINETIME_BOARD_H
