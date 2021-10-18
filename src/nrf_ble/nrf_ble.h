#ifndef _NRF_BLE_H
#define _NRF_BLE_H

#include <stdbool.h>
#include <stdint.h>

void nrf_ble_init();
void send_data_ble(uint8_t * data, uint16_t size);

#endif // _NRF_BLE_H