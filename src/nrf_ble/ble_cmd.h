#ifndef BLE_CMD_H
#define BLE_CMD_H

#include <stdbool.h>
#include <stdint.h>
#include "nordic_common.h"
#include "nrf_soc.h"

#define COMMAND_BASE 0x00
#define COMMAND_TIME_UPDATE            COMMAND_BASE + 0x01
#define COMMAND_NOTIFICATION           COMMAND_BASE + 0x02
#define COMMAND_DELETE_NOTIFICATION    COMMAND_BASE + 0x03
#define COMMAND_SET_ALARMS             COMMAND_BASE + 0x04
#define COMMAND_SET_CALL               COMMAND_BASE + 0x05
#define COMMAND_SET_MUSIC              COMMAND_BASE + 0x06
#define COMMAND_SET_MUSIC_INFO         COMMAND_BASE + 0x07
#define COMMAND_ACTIVITY_STATUS        COMMAND_BASE + 0x08
#define COMMAND_FIND_DEVICE            COMMAND_BASE + 0x09
#define COMMAND_VIBRATION              COMMAND_BASE + 0x0a
#define COMMAND_WEATHER                COMMAND_BASE + 0x0b

#define COMMAND_PT_VERSION             COMMAND_BASE + 0x01
#define COMMAND_PT_BATTERY             COMMAND_BASE + 0x02

void ble_command(uint8_t msg_type);
void ble_connection(void);
void ble_update(void);

typedef struct _notification {
    uint32_t    id;
    char *      subject;
    char *      body;
    uint8_t     type;
} notification_t;

typedef struct _weather {
    int8_t      currentTemp;
    uint8_t     currentHumidity;
    int8_t      todayMaxTemp;
    int8_t      todayMinTemp;
    char *      location;
    char *      currentCondition;
    bool        hasData;
    bool        newData;
} weather_t;

struct pinetimecOSBle {
    bool    find_device;
    weather_t weather;
    notification_t notification;
};

struct pinetimecOSBle pinetimecosBLE;

#endif //BLE_CMD_H
