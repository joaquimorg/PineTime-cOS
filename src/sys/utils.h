#ifndef UTILS_H
#define UTILS_H

#include <stdbool.h>
#include <stdint.h>
#include "nordic_common.h"
#include "nrf_soc.h"

#   define container_of(PTR, TYPE, MEMBER) \
        ((TYPE *) ((char *) (PTR) - offsetof(TYPE, MEMBER)))

static inline bool in_isr(void) {
    return (SCB->ICSR & SCB_ICSR_VECTACTIVE_Msk) != 0;
}

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

void display_on(void);
void display_off(void);

const char* actual_reset_reason(void);

void ble_command(uint8_t msg_type);

#endif //UTILS_H