#include <stdbool.h>
#include <stdint.h>
#include <string.h>

#include "pinetime_board.h"

#include "cst816.h"
#include "nrf_gpio.h"
#include "nrf_drv_twi.h"
#include "nrf_delay.h"

NRF_TWIM_Type *twiBaseAddress;

void twi_init(void)
{
  NRF_GPIO->PIN_CNF[TWI_SCL] = ((uint32_t)GPIO_PIN_CNF_DIR_Input      << GPIO_PIN_CNF_DIR_Pos)
                                  | ((uint32_t)GPIO_PIN_CNF_INPUT_Connect    << GPIO_PIN_CNF_INPUT_Pos)
                                  | ((uint32_t)GPIO_PIN_CNF_PULL_Pullup      << GPIO_PIN_CNF_PULL_Pos)
                                  | ((uint32_t)GPIO_PIN_CNF_DRIVE_S0D1       << GPIO_PIN_CNF_DRIVE_Pos)
                                  | ((uint32_t)GPIO_PIN_CNF_SENSE_Disabled   << GPIO_PIN_CNF_SENSE_Pos);

  NRF_GPIO->PIN_CNF[TWI_SDA] = ((uint32_t)GPIO_PIN_CNF_DIR_Input        << GPIO_PIN_CNF_DIR_Pos)
                                  | ((uint32_t)GPIO_PIN_CNF_INPUT_Connect    << GPIO_PIN_CNF_INPUT_Pos)
                                  | ((uint32_t)GPIO_PIN_CNF_PULL_Pullup      << GPIO_PIN_CNF_PULL_Pos)
                                  | ((uint32_t)GPIO_PIN_CNF_DRIVE_S0D1       << GPIO_PIN_CNF_DRIVE_Pos)
                                  | ((uint32_t)GPIO_PIN_CNF_SENSE_Disabled   << GPIO_PIN_CNF_SENSE_Pos);
  twiBaseAddress = NRF_TWIM1;

  twiBaseAddress->FREQUENCY = TWIM_FREQUENCY_FREQUENCY_K250;

  twiBaseAddress->PSEL.SCL = TWI_SCL;
  twiBaseAddress->PSEL.SDA = TWI_SDA;
  twiBaseAddress->EVENTS_LASTRX = 0;
  twiBaseAddress->EVENTS_STOPPED = 0;
  twiBaseAddress->EVENTS_LASTTX = 0;
  twiBaseAddress->EVENTS_ERROR = 0;
  twiBaseAddress->EVENTS_RXSTARTED = 0;
  twiBaseAddress->EVENTS_SUSPENDED = 0;
  twiBaseAddress->EVENTS_TXSTARTED = 0;

  twiBaseAddress->ENABLE = (TWIM_ENABLE_ENABLE_Enabled << TWIM_ENABLE_ENABLE_Pos);

}

void twiRead(uint8_t deviceAddress, uint8_t *buffer, size_t size, bool stop)
{
  twiBaseAddress->ADDRESS = deviceAddress;
  twiBaseAddress->TASKS_RESUME = 0x1UL;
  twiBaseAddress->RXD.PTR = (uint32_t)buffer;
  twiBaseAddress->RXD.MAXCNT = size;

  twiBaseAddress->TASKS_STARTRX = 1;

  while (!twiBaseAddress->EVENTS_RXSTARTED && !twiBaseAddress->EVENTS_ERROR)
    ;
  twiBaseAddress->EVENTS_RXSTARTED = 0x0UL;

  while (!twiBaseAddress->EVENTS_LASTRX && !twiBaseAddress->EVENTS_ERROR)
    ;
  twiBaseAddress->EVENTS_LASTRX = 0x0UL;

  if (stop || twiBaseAddress->EVENTS_ERROR)
  {
    twiBaseAddress->TASKS_STOP = 0x1UL;
    while (!twiBaseAddress->EVENTS_STOPPED)
      ;
    twiBaseAddress->EVENTS_STOPPED = 0x0UL;
  }
  else
  {
    twiBaseAddress->TASKS_SUSPEND = 0x1UL;
    while (!twiBaseAddress->EVENTS_SUSPENDED)
      ;
    twiBaseAddress->EVENTS_SUSPENDED = 0x0UL;
  }

  if (twiBaseAddress->EVENTS_ERROR)
  {
    twiBaseAddress->EVENTS_ERROR = 0x0UL;
  }
}

void twiWrite(uint8_t deviceAddress, const uint8_t *data, size_t size, bool stop)
{
  twiBaseAddress->ADDRESS = deviceAddress;
  twiBaseAddress->TASKS_RESUME = 0x1UL;
  twiBaseAddress->TXD.PTR = (uint32_t)data;
  twiBaseAddress->TXD.MAXCNT = size;

  twiBaseAddress->TASKS_STARTTX = 1;

  while (!twiBaseAddress->EVENTS_TXSTARTED && !twiBaseAddress->EVENTS_ERROR)
    ;
  twiBaseAddress->EVENTS_TXSTARTED = 0x0UL;

  //NRFX_DELAY_US(10);
  while (!twiBaseAddress->EVENTS_LASTTX && !twiBaseAddress->EVENTS_ERROR)
    ;

  twiBaseAddress->EVENTS_LASTTX = 0x0UL;

  if (stop || twiBaseAddress->EVENTS_ERROR)
  {
    twiBaseAddress->TASKS_STOP = 0x1UL;
    while (!twiBaseAddress->EVENTS_STOPPED)
      ;
    twiBaseAddress->EVENTS_STOPPED = 0x0UL;
  }
  else
  {
    twiBaseAddress->TASKS_SUSPEND = 0x1UL;
    //NRFX_DELAY_US(5);
    while (!twiBaseAddress->EVENTS_SUSPENDED)
      ;

    twiBaseAddress->EVENTS_SUSPENDED = 0x0UL;
  }

  if (twiBaseAddress->EVENTS_ERROR)
  {
    twiBaseAddress->EVENTS_ERROR = 0x0UL;
    uint32_t error = twiBaseAddress->ERRORSRC;
    twiBaseAddress->ERRORSRC = error;
  }
}

void i2c_Read(uint8_t deviceAddress, uint8_t registerAddress, uint8_t *data, size_t size)
{
  twiWrite(deviceAddress, &registerAddress, 1, false);
  twiRead(deviceAddress, data, size, true);
}

void i2c_Write(uint8_t deviceAddress, uint8_t registerAddress, const uint8_t *data, size_t size)
{
  uint8_t internalBuffer[128];
  internalBuffer[0] = registerAddress;
  memcpy(internalBuffer + 1, data, size);
  twiWrite(deviceAddress, internalBuffer, size + 1, true);
}

void cst816Init(void)
{
  twi_init();

  //ret_code_t err_code;
  nrf_gpio_cfg_output(TP_RST);
  nrf_gpio_pin_set(TP_RST);
  nrf_delay_ms(50);
  nrf_gpio_pin_clear(TP_RST);
  nrf_delay_ms(5);
  nrf_gpio_pin_set(TP_RST);
  nrf_delay_ms(50);

  tsData.version15 = 0x01;
  i2c_Write(TP_TWI_ADDR, 0xD0, &tsData.version15, 1);
  nrf_delay_ms(50);

  i2c_Read(TP_TWI_ADDR, 0x15, &tsData.version15, 1);
  nrf_delay_ms(5);
  i2c_Read(TP_TWI_ADDR, 0xa7, tsData.versionInfo, 3);
}

void cst816Get(void)
{
  uint8_t touchBuffer[8];

  i2c_Read(TP_TWI_ADDR, 0x01, touchBuffer, 6);

  tsData.gesture = touchBuffer[0];
  tsData.touchpoints = touchBuffer[1];
  tsData.event = touchBuffer[2] >> 6;
  tsData.xpos = touchBuffer[3];
  tsData.ypos = touchBuffer[5];
  if (tsData.xpos == 255 && tsData.ypos == 255)
  {
    tsData.xpos = tsData.last_xpos;
    tsData.ypos = tsData.last_ypos;
  }
  else
  {
    tsData.last_xpos = tsData.xpos;
    tsData.last_ypos = tsData.ypos;
  }

  /*i2c_Read(TP_TWI_ADDR, 0xa5, touchBuffer, 1);*/
  //tsData.gesture = touchBuffer[9];
}