#include "st7789.h"
#include "pinetime_board.h"
#include "nrf_delay.h"
#include "nrf_gpio.h"
#include "spi_master2.h"


#define NOP         0x00
#define SWRESET     0x01
#define SLPIN		    0x10
#define SLPOUT      0x11
#define NORON       0x13
//#define INVOFF      0x20
#define INVON       0x21
#define DISPOFF     0x28
#define DISPON      0x29
#define CASET       0x2a
#define RASET       0x2b
#define RAMWR       0x2c
#define VSCRDEF     0x33
//#define TEON        0x34
#define TEOFF       0x35
#define COLMOD      0x3a
#define MADCTL      0x36
#define VSCSAD      0x37
#define PORCTRL     0xB2  // Porch Setting
#define GCTRL       0xB7    // Gate Contro
#define VCOMS       0xBB    // VCOMS Setting
#define LCMCTRL     0xC0  // LCM Control
#define VDVVRHEN    0xC2 // VDV and VRH Command Enabl
#define VRHS        0xC3
#define VDVS        0xC4
#define FRCTRL2     0xC6
#define PWCTRL1     0xD0
#define PVGAMCTRL   0xE0
#define NVGAMCTRL   0xE1
#define RAMCTRL     0xB0 // RAM control

struct st7789_cmd {
  uint8_t cmd;
  const uint8_t *data;
  uint8_t len;
};


uint16_t verticalScrollingStartAddress = 0;

const static struct st7789_cmd st7789_init_data[] = {
    //{SWRESET, NULL},
    //{SLPOUT, NULL},
    {COLMOD, (uint8_t *)"\x55", 1}, // MCU will send 16-bit RGB565 = 55 / 12-bit = 53
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

void st7789_display_off(void) {
  st7789_send(DISPOFF, NULL, 0);
}


void st7789_display_on(void) {
  st7789_send(DISPON, NULL, 0);
}

void st7789_sleep_out(void) {
  st7789_send(SLPOUT, NULL, 0);
  nrf_delay_ms(10);
}

void st7789_sleep_in(void) {
  st7789_send(SLPIN, NULL, 0);
  nrf_delay_ms(10);
}

void st7789_vertical_scroll_start_address(uint16_t line) {
  verticalScrollingStartAddress = line;
  //assert(line < 320);

  uint8_t temp[2];

  temp[0] = (line >> 8u);
  temp[1] = (line & 0x00ffu);
  
  st7789_send(VSCSAD, temp, 2);

}

void st7789_vertical_scroll_definition(uint16_t topFixedLines, uint16_t scrollLines, uint16_t bottomFixedLines) {
  uint8_t temp[6];
  temp[0] = (topFixedLines >> 8u);
  temp[1] = (topFixedLines & 0x00ffu);
  temp[2] = (scrollLines >> 8u);
  temp[3] = (scrollLines & 0x00ffu);
  temp[4] = (bottomFixedLines >> 8u);
  temp[5] = (bottomFixedLines & 0x00ffu);
  st7789_send(VSCRDEF, temp, 6);
}

void st7789_row_address_set() {
  uint8_t temp[4];
  temp[0] = 0x00;
  temp[1] = 0x00;
  temp[2] = (320u >> 8u);
  temp[3] = (320u & 0xffu);
  st7789_send(RASET, temp, 4);
}

void st7789_sleep(void) {
  st7789_sleep_in();
  nrf_delay_ms(50);
  //nrf_gpio_cfg_default(pinDataCommand);  
}

void st7789_wake_up(void) {
  //nrf_gpio_cfg_output(pinDataCommand);
  //nrf_delay_ms(50);
  st7789_sleep_out();
  st7789_vertical_scroll_definition(0, 320, 0);
  st7789_vertical_scroll_start_address(verticalScrollingStartAddress);
  st7789_display_on();
}


void st7789_init(void) {

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
    st7789_row_address_set();

    st7789_vertical_scroll_definition(0, 320, 0);
    st7789_vertical_scroll_start_address(0);

    /* enable the display */
    st7789_send(DISPON, NULL, 0);

    st7789_fill(0, 0, 240, 240, RGB2COLOR(0, 0, 0));
}

