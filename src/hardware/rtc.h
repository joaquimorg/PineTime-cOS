#ifndef RTC_H
#define RTC_H

#include "stdint.h"
#include "stdbool.h"
#include "string.h"
#include <stdio.h>

typedef struct
{
    uint16_t    year;     // 2000+
    uint8_t     month;    // 1-12
    uint8_t     day;      // 1-31
    uint8_t     hour;     // 0-23
    uint8_t     minutes;  // 0-59
    uint8_t     seconds;  // 0-59
    uint8_t     week;     // 0-6
    
} UTCTimeStruct;

void rtc_init(void);

void rtc_set_time(uint32_t timestampNow);
uint32_t rtc_get_time(void);

void get_UTC_time(UTCTimeStruct *tm);

const char *get_months_low( uint8_t month );
const char *get_days_low( uint8_t day );
const char *get_days_low_short( uint8_t day );

const char *get_months( uint8_t month );
const char *get_days( uint8_t day );

#endif //RTC_H