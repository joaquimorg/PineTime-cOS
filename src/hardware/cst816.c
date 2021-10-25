#include <stdbool.h>
#include <stdint.h>
#include <string.h>

#include "pinetime_board.h"

#include "cst816.h"
#include "nrf_gpio.h"
#include "nrf_drv_twi.h"
#include "nrf_delay.h"


/* TWI instance ID. */
#define TWI_INSTANCE_ID     1
/* TWI instance. */
static const nrf_drv_twi_t m_twi = NRF_DRV_TWI_INSTANCE(TWI_INSTANCE_ID);


void twi_init(void) {

    ret_code_t err_code;

    const nrf_drv_twi_config_t twi_cst816_config = {
        .scl                = TWI_SCL,
        .sda                = TWI_SDA,
        .frequency          = NRF_DRV_TWI_FREQ_400K,
        .interrupt_priority = APP_IRQ_PRIORITY_HIGH,
        .clear_bus_init     = false
    };

    err_code = nrf_drv_twi_init(&m_twi, &twi_cst816_config, NULL, NULL);
    APP_ERROR_CHECK(err_code);

    nrf_drv_twi_enable(&m_twi);
    
}


void i2c_read(uint8_t deviceAddress, uint8_t registerAddress, uint8_t *data, size_t size) {
    nrf_drv_twi_tx(&m_twi, deviceAddress, &registerAddress, 1, false);
    nrf_drv_twi_rx(&m_twi, deviceAddress, data, size);
}

void i2c_write(uint8_t deviceAddress, uint8_t registerAddress, const uint8_t *data, size_t size) {
    nrf_drv_twi_tx(&m_twi, deviceAddress, &registerAddress, 1, false);
    nrf_drv_twi_tx(&m_twi, deviceAddress, data, size, true);
}

void cst816Init(void) {
  twi_init();

  //ret_code_t err_code;
  nrf_gpio_cfg_output(TP_RST);
  nrf_gpio_pin_set(TP_RST);
  nrf_delay_ms(15);
  nrf_gpio_pin_clear(TP_RST);
  nrf_delay_ms(5);
  nrf_gpio_pin_set(TP_RST);
  nrf_delay_ms(15);

  tsData.version15 = 0x01;
  i2c_write(TP_TWI_ADDR, 0xD0, &tsData.version15, 1);
  nrf_delay_ms(15);

  i2c_read(TP_TWI_ADDR, 0x15, &tsData.version15, 1);
  nrf_delay_ms(5);
  i2c_read(TP_TWI_ADDR, 0xa7, tsData.versionInfo, 3);

  nrf_delay_ms(15);

  /*
  [2] EnConLR - Continuous operation can slide around
  [1] EnConUD - Slide up and down to enable continuous operation
  [0] EnDClick - Enable Double-click action
  */
  const uint8_t motionMask = 0b00000001;
  i2c_write(TP_TWI_ADDR, 0xEC, &motionMask, 1);

  /*
  [7] EnTest interrupt pin test, and automatically send out low pulse periodically after being enabled.
  [6] When EnTouch detects a touch, it periodically sends out low pulses.
  [5] When EnChange detects a touch state change, it sends out a low pulse.
  [4] When EnMotion detects a gesture, it sends out a low pulse.
  [0] OnceWLP Long press gesture only sends out a low pulse signal.
  */
  const uint8_t irqCtl = 0b00010000;
  i2c_write(TP_TWI_ADDR, 0xFA, &irqCtl, 1);

}

void cst816Get(void) {

  uint8_t touchBuffer[8];

  i2c_read(TP_TWI_ADDR, 0x01, touchBuffer, 6);

  tsData.gesture = touchBuffer[0];
  tsData.touchpoints = touchBuffer[1];
  tsData.event = touchBuffer[2] >> 6;
  tsData.xpos = touchBuffer[3];
  tsData.ypos = touchBuffer[5];
  if (tsData.xpos == 255 && tsData.ypos == 255) {
    tsData.xpos = tsData.last_xpos;
    tsData.ypos = tsData.last_ypos;
  } else {
    tsData.last_xpos = tsData.xpos;
    tsData.last_ypos = tsData.ypos;
  }
  
}