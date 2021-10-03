#ifndef PINETIME_BOARD_H
#define PINETIME_BOARD_H

#include "nrf_delay.h"
#include "nrf_gpio.h"
#include "spi_master2.h"

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

// ------------------------------------------------------------------------------------------------------
// LCD


#define RGB2COLOR(r, g, b) ((((r >> 3) << 11) | ((g >> 2) << 5) | (b >> 3)))

#define ___ 0
#define __X 1
#define _X_ 2
#define _XX 3
#define X__ 4
#define X_X 5
#define XX_ 6
#define XXX 7
#define PACK_5_TO_16(A,B,C,D,E) ((A) | (B<<3) | (C<<6) | (D<<9) | (E<<12))
 // 48

#define LCD_FONT_3X5_CHARS 95
const unsigned short LCD_FONT_3X5[] = { // from 33 up to 127
    PACK_5_TO_16( XXX , _X_ , XXX , XX_ , X_X ), // 01234
    PACK_5_TO_16( X_X , XX_ , __X , __X , X_X ),
    PACK_5_TO_16( X_X , _X_ , _X_ , XX_ , XXX ),
    PACK_5_TO_16( X_X , _X_ , X__ , __X , __X ),
    PACK_5_TO_16( XXX , XXX , XXX , XX_ , __X ),

    PACK_5_TO_16( XXX , XXX , XXX , XXX , XXX ), // 56789
    PACK_5_TO_16( X__ , X__ , __X , X_X , X_X ),
    PACK_5_TO_16( XXX , XXX , _X_ , XXX , XXX ),
    PACK_5_TO_16( __X , X_X , _X_ , X_X , __X ),
    PACK_5_TO_16( XXX , XXX , _X_ , XXX , XXX ),

    PACK_5_TO_16( ___ , ___ , __X , ___ , X__ ), // :;<=>
    PACK_5_TO_16( _X_ , _X_ , _X_ , XXX , _X_ ),
    PACK_5_TO_16( ___ , ___ , X__ , ___ , __X ),
    PACK_5_TO_16( _X_ , _X_ , _X_ , XXX , _X_ ),
    PACK_5_TO_16( ___ , X__ , __X , ___ , X__ ),

    PACK_5_TO_16( _X_ , ___ , _X_ , XX_ , _XX ), // ?@ABC
    PACK_5_TO_16( X_X , _X_ , X_X , X_X , X__ ), // @ is used as +
    PACK_5_TO_16( __X , XXX , XXX , XX_ , X__ ),
    PACK_5_TO_16( ___ , _X_ , X_X , X_X , X__ ),
    PACK_5_TO_16( _X_ , ___ , X_X , XX_ , _XX ),

    PACK_5_TO_16( XX_ , XXX , XXX , _XX , X_X ), // DEFGH
    PACK_5_TO_16( X_X , X__ , X__ , X__ , X_X ),
    PACK_5_TO_16( X_X , XX_ , XXX , X_X , XXX ),
    PACK_5_TO_16( X_X , X__ , X__ , X_X , X_X ),
    PACK_5_TO_16( XX_ , XXX , X__ , _XX , X_X ),

    PACK_5_TO_16( XXX , XXX , X_X , X__ , X_X ), // IJKLM
    PACK_5_TO_16( _X_ , __X , X_X , X__ , XXX ),
    PACK_5_TO_16( _X_ , __X , XX_ , X__ , XXX ),
    PACK_5_TO_16( _X_ , __X , X_X , X__ , X_X ),
    PACK_5_TO_16( XXX , XX_ , X_X , XXX , X_X ),

    PACK_5_TO_16( XX_ , _XX , XX_ , _X_ , XX_ ), // NOPQR
    PACK_5_TO_16( X_X , X_X , X_X , X_X , X_X ),
    PACK_5_TO_16( X_X , X_X , XX_ , X_X , XX_ ),
    PACK_5_TO_16( X_X , X_X , X__ , X_X , X_X ),
    PACK_5_TO_16( X_X , _X_ , X__ , _XX , X_X ),

    PACK_5_TO_16( _XX , XXX , X_X , X_X , X_X ), // STUVW
    PACK_5_TO_16( X__ , _X_ , X_X , X_X , X_X ),
    PACK_5_TO_16( _X_ , _X_ , X_X , X_X , XXX ),
    PACK_5_TO_16( __X , _X_ , X_X , _X_ , XXX ),
    PACK_5_TO_16( XX_ , _X_ , _X_ , _X_ , X_X ),

    PACK_5_TO_16( X_X , X_X , XXX , _XX , ___ ), // XYZ[\ end
    PACK_5_TO_16( X_X , X_X , __X , _X_ , ___ ), // \ is used as .
    PACK_5_TO_16( _X_ , _X_ , _X_ , _X_ , ___ ),
    PACK_5_TO_16( X_X , _X_ , X__ , _X_ , ___ ),
    PACK_5_TO_16( X_X , _X_ , XXX , _XX , _X_ ),
};

#define NOP 0x00
#define SWRESET 0x01
#define SLPOUT 0x11
#define NORON 0x13
#define INVOFF 0x20
#define INVON 0x21
#define DISPOFF 0x28
#define DISPON 0x29
#define CASET 0x2a
#define RASET 0x2b
#define RAMWR 0x2c
#define TEON 0x34
#define TEOFF 0x35
#define COLMOD 0x3a
#define MADCTL 0x36
#define PORCTRL 0xB2  // Porch Setting
#define GCTRL 0xB7    // Gate Contro
#define VCOMS 0xBB    // VCOMS Setting
#define LCMCTRL 0xC0  // LCM Control
#define VDVVRHEN 0xC2 // VDV and VRH Command Enabl
#define VRHS 0xC3
#define VDVS 0xC4
#define FRCTRL2 0xC6
#define PWCTRL1 0xD0
#define PVGAMCTRL 0xE0
#define NVGAMCTRL 0xE1
#define RAMCTRL 0xB0 // RAM control

struct st7789_cmd {
  uint8_t cmd;
  const uint8_t *data;
  uint8_t len;
};

const static struct st7789_cmd st7789_init_data[] = {
    //{SWRESET, NULL},
    //{SLPOUT, NULL},
    {COLMOD, (uint8_t *)"\x55", 1}, // MCU will send 16-bit RGB565
    {MADCTL, (uint8_t *)"\x00", 1}, // Left to right, top to bottom
    {PORCTRL, (uint8_t *)"\x0C\x0C\x00\x33\x33", 5},
    {GCTRL, (uint8_t *)"\x45", 1},
    {VCOMS, (uint8_t *)"\x2b", 1},
    {LCMCTRL, (uint8_t *)"\x2c", 1},
    {VDVVRHEN, (uint8_t *)"\x01\xFF", 2},
    {VRHS, (uint8_t *)"\x11", 1},
    {VDVS, (uint8_t *)"\x20", 1},
    {FRCTRL2, (uint8_t *)"\x0F", 1},
    {PWCTRL1, (uint8_t *)"\xA4\xA1", 2},
    {PVGAMCTRL, (uint8_t *)"\xd0\x00\x05\x0e\x15\x0d\x37\x43\x47\x09\x15\x12\x16\x19", 14},
    {NVGAMCTRL, (uint8_t *)"\xd0\x00\x05\x0d\x0c\x06\x2d\x44\x40\x0e\x1c\x18\x16\x19", 14},
    //{RAMCTRL, (uint8_t *)"\x00\xf8", 2},
    //{TEOFF, NULL},
    {INVON, NULL},
    {NORON, NULL},
    {NOP, NULL},
};

/*
static ret_code_t spi_init(void)
{
    ret_code_t err_code;

    nrf_drv_spi_config_t spi_config = NRF_DRV_SPI_DEFAULT_CONFIG;

    spi_config.sck_pin    = SPI_SCK;
    spi_config.miso_pin   = SPI_MISO;
    spi_config.mosi_pin   = SPI_MOSI;
    spi_config.ss_pin     = LCD_CSN;
    spi_config.frequency  = NRF_DRV_SPI_FREQ_8M;
    spi_config.mode       = NRF_DRV_SPI_MODE_3;

    err_code = nrf_drv_spi_init(&spi, &spi_config, NULL, NULL);
    return err_code;
}

void spi_write(const void * data, size_t size)
{
    //APP_ERROR_CHECK(nrf_drv_spi_transfer(&spi, data, size, NULL, 0));
    nrf_drv_spi_transfer(&spi, data, size, NULL, 0);
}

*/

#define LCD_SPI_SCK_SET() (*(volatile uint32_t*)0x50000508)=1<<SPI_SCK
#define LCD_SPI_SCK_CLEAR() (*(volatile uint32_t*)0x5000050C)=1<<SPI_SCK
#define LCD_SPI_MOSI_SET() (*(volatile uint32_t*)0x50000508)=1<<SPI_MOSI
#define LCD_SPI_MOSI_CLEAR() (*(volatile uint32_t*)0x5000050C)=1<<SPI_MOSI

void spi_wr(int data) {
    for (int bit=7;bit>=0;bit--) {
        LCD_SPI_SCK_CLEAR();
        if ((data>>bit)&1) LCD_SPI_MOSI_SET();
        else LCD_SPI_MOSI_CLEAR();
        LCD_SPI_SCK_SET();
    }
}

void st7789_send(uint8_t cmd, const uint8_t *data, unsigned len) {

    nrf_gpio_pin_clear(LCD_CSN);
    if (cmd != NOP) {
        nrf_gpio_pin_clear(LCD_DC); // command
        spi_master_tx(SPI0, 1, &cmd);
    }
    if (data) {
        nrf_gpio_pin_set(LCD_DC); // data
        spi_master_tx(SPI0, len, data);
    }
    nrf_gpio_pin_set(LCD_CSN);

}

static void st7789_init(void) {
    
    nrf_gpio_pin_set(LCD_DC);
    nrf_gpio_pin_set(LCD_CSN);
    //nrf_gpio_pin_set(SPI_MOSI);
    //nrf_gpio_pin_set(SPI_SCK);

    nrf_gpio_cfg_output(LCD_DC);
    nrf_gpio_cfg_output(LCD_CSN);
    //nrf_gpio_cfg_output(SPI_MOSI);
    //nrf_gpio_cfg_output(SPI_SCK);

    SPIConfig_t lcd_spi = {
        .Config.Fields.BitOrder = SPI_BITORDER_MSB_LSB,
        .Config.Fields.Mode     = SPI_MODE3,
        .Frequency              = SPI_FREQ_8MBPS,
        .Pin_SCK                = SPI_SCK,
        .Pin_MOSI               = SPI_MOSI,
        .Pin_CSN                = LCD_CSN
    };

    spi_master_init(SPI0, &lcd_spi);

    /* deliver a reset */
    nrf_gpio_pin_clear(LCD_RST);
    nrf_gpio_cfg_output(LCD_RST);
    nrf_delay_ms(10);
    nrf_gpio_pin_set(LCD_RST);
    nrf_delay_ms(50);

    /* initialize the display */
    st7789_send(SWRESET, NULL, 0);
    nrf_delay_ms(20);

    st7789_send(SLPOUT, NULL, 0);
    nrf_delay_ms(10);

    for (const struct st7789_cmd *i = st7789_init_data; i->cmd != NOP; i++) {
        st7789_send(i->cmd, i->data, i->len);
    }

    /* enable the display */
    st7789_send(DISPON, NULL, 0);
}


void st7789_set_window(uint32_t x, uint32_t y, uint32_t w, uint32_t h) {
  uint8_t temp[4];
  //y += 80; // when rotated screen

  temp[0] = (x >> 8);
  temp[1] = x;
  temp[2] = ((x + w - 1) >> 8);
  temp[3] = (x + w - 1);
  st7789_send(CASET, temp, 4);

  temp[0] = (y >> 8);
  temp[1] = y;
  temp[2] = ((y + h - 1) >> 8);
  temp[3] = ((y + h - 1) & 0xFF);
  st7789_send(RASET, temp, 4);

  st7789_send(RAMWR, NULL, 0);
}

void st7789_fill(uint16_t x1, uint16_t y1, uint16_t x2, uint16_t y2, uint16_t color) {
  uint16_t i;

  uint8_t linebuffer[2 * 240];
  uint16_t bp = 0;

  st7789_set_window(x1, y1, x2, y2);

  nrf_gpio_pin_clear(LCD_CSN);
  nrf_gpio_pin_set(LCD_DC);

  for (i = y1; i < y2; i++) {
    uint8_t rl = (x2 - x1);
    while (rl) {
      linebuffer[bp] = color >> 8;
      linebuffer[bp + 1] = color & 0xff;
      bp += 2;
      rl -= 1;

      if (bp >= sizeof(linebuffer)) {
        //st7789_send(NOP, linebuffer, sizeof(linebuffer));
        spi_master_tx(SPI0, sizeof(linebuffer), linebuffer);
        bp = 0;
      }
    }
  }
  nrf_gpio_pin_set(LCD_CSN);
}


#define CHAR_ZOOM 4 

void st7789_pixel(uint16_t x1, uint16_t y1, uint16_t color) {
    uint8_t buffer[(CHAR_ZOOM*CHAR_ZOOM) * 2];
    uint8_t buffpos = 0;
    st7789_set_window(x1, y1, CHAR_ZOOM, CHAR_ZOOM);    
    for (uint8_t size=0; size < CHAR_ZOOM*CHAR_ZOOM; size++) {
        buffer[buffpos++] = color >> 8;
        buffer[buffpos++] = color & 0xff;
    }
    st7789_send(NOP, buffer, sizeof(buffer));
}

void lcd_char(int x1, int y1, char ch, uint16_t color) {
    // char replacements so we don't waste font space
    if (ch=='.') ch='\\';
    if (ch=='+') ch='@';
    if (ch>='a') ch-='a'-'A';
    int idx = ch - '0';
    if (idx<0 || idx>=LCD_FONT_3X5_CHARS) return; // no char for this - just return
    int cidx = idx % 5; // character index
    idx -= cidx;
    int y;
    for (y=0;y<5;y++) {
        unsigned short line = LCD_FONT_3X5[idx + y] >> (cidx*3);
        if (line&4) st7789_pixel(x1+0, (y * CHAR_ZOOM)+y1, color);
        else  st7789_pixel(x1+0, (y * CHAR_ZOOM)+y1, RGB2COLOR(0, 0, 0));
        
        if (line&2) st7789_pixel(x1+CHAR_ZOOM, (y * CHAR_ZOOM)+y1, color);
        else  st7789_pixel(x1+CHAR_ZOOM, (y * CHAR_ZOOM)+y1, RGB2COLOR(0, 0, 0));

        if (line&1) st7789_pixel(x1+CHAR_ZOOM*2, (y * CHAR_ZOOM)+y1, color);
        else  st7789_pixel(x1+CHAR_ZOOM*2, (y * CHAR_ZOOM)+y1, RGB2COLOR(0, 0, 0));
    }
}

void lcd_print(int x1, int y1, char *ch, uint16_t color) {
  while (*ch) {
    lcd_char(x1, y1, *ch, color);
    x1 += CHAR_ZOOM*CHAR_ZOOM;
    ch++;
  }
}

// ------------------------------------------------------------------------------------------------------
//
static void hardware_init(void) {

    // Button Enable
    nrf_gpio_pin_set(KEY_ENABLE);
    nrf_gpio_cfg_output(KEY_ENABLE);

    // Backlight
    nrf_gpio_pin_clear(LCD_LIGHT_1);
    nrf_gpio_cfg_output(LCD_LIGHT_1);

    st7789_init();

    st7789_fill(0, 0, 240, 240, RGB2COLOR(0, 0, 0));

}

#endif // PINETIME_BOARD_H
