#include <nrf.h>
#include <string.h>
#include <stdint.h>
#include <nrf_gpio.h>
#include "flash.h"
#include "pinetime_board.h"

static void spi_write(uint8_t *data, int length) {
    NRF_SPIM0->TXD.MAXCNT = length;
    NRF_SPIM0->TXD.PTR = (uint32_t)data;

    NRF_SPIM0->EVENTS_END = 0;
    NRF_SPIM0->TASKS_START = 1;
    while(NRF_SPIM0->EVENTS_END == 0) __NOP();
}

static void spi_write_byte(uint8_t byte) {
    nrf_gpio_pin_write(FLASH_CSN, 0);
    spi_write(&byte, 1);
    nrf_gpio_pin_write(FLASH_CSN, 1);
}

static void spi_read(volatile uint8_t *data, int length) {
    NRF_SPIM0->RXD.MAXCNT = length;
    NRF_SPIM0->RXD.PTR = (uint32_t)data;

    NRF_SPIM0->EVENTS_END = 0;
    NRF_SPIM0->TASKS_START = 1;
    while(NRF_SPIM0->EVENTS_END == 0) __NOP();
    NRF_SPIM0->RXD.MAXCNT = 0;
}

uint16_t check_status() {
    nrf_gpio_pin_write(FLASH_CSN, 0);
    uint8_t data[2];
    uint8_t cmd = FLASH_RDSR;
    spi_write(&cmd, 1);
    spi_read(data, 2);
    nrf_gpio_pin_write(FLASH_CSN, 1);
    return (data[0] + (data[1] << 8));
}

uint8_t check_short_status() {
    nrf_gpio_pin_write(FLASH_CSN, 0);
    uint8_t data;
    uint8_t cmd = FLASH_RDSR;
    spi_write(&cmd, 1);
    spi_read(&data, 1);
    nrf_gpio_pin_write(FLASH_CSN, 1);
    return (data);
}

void spiflash_sector_erase(uint32_t addr) {
    spi_write_byte(FLASH_WREN);

    nrf_gpio_pin_write(FLASH_CSN,0);
    uint8_t cmd[] = {FLASH_SE, addr >> 16, addr >> 8, addr};
    spi_write(cmd, 4);
    nrf_gpio_pin_write(FLASH_CSN, 1);
    while (check_short_status() & 1);
}

void spiflash_write_data(uint32_t addr, uint8_t* data, uint32_t length) {
    while (length > 0) {
        spi_write_byte(FLASH_WREN);

        nrf_gpio_pin_write(FLASH_CSN,0);

        uint8_t cmd[0x80 + 4];
        cmd[0] = FLASH_PP; cmd[1] = addr >> 16; cmd[2] = addr >> 8; cmd[3] = addr;

        int writeLen = length > 0x80 ? 0x80 : length; // not the fastest transfer, but it works

        memcpy(&cmd[4], data, writeLen);
        spi_write(cmd, sizeof(cmd));

        length -= writeLen;
        addr += writeLen;
        data += writeLen;

        nrf_gpio_pin_write(FLASH_CSN, 1);

        while (check_short_status() & 1); // potential optimization here, potentially unneccesary idle time
    }
}

void spiflash_read_data(uint32_t addr, uint8_t* data, uint32_t length) {
    while (length > 0) {
        nrf_gpio_pin_write(FLASH_CSN,0);

        uint8_t cmd[] = {FLASH_READ, addr >> 16, addr >> 8, addr};
        int readLen = length > 0xff ? 0xff : length;

        spi_write(cmd, sizeof(cmd));
        spi_read(data, readLen);

        length -= readLen;
        addr += readLen;
        data += readLen;

        nrf_gpio_pin_write(FLASH_CSN, 1);
    }
}

void spiflash_init() {
    nrf_gpio_cfg_output(FLASH_CSN);
    nrf_gpio_pin_write(FLASH_CSN, 1);
}

void spiflash_sleep() {
    nrf_gpio_pin_write(FLASH_CSN, 0);
    spi_write_byte(FLASH_DPD);
    nrf_gpio_pin_write(FLASH_CSN, 1);
}

void spiflash_wakeup() {
    nrf_gpio_pin_write(FLASH_CSN, 0);

    uint8_t data[4] = {FLASH_RDPD, 0x01, 0x02, 0x03};    
    spi_write(data, 4);
    spi_read(data, 3);

    nrf_gpio_pin_write(FLASH_CSN, 1);
}