#include "nrf_drv_rtc.h"
#include "nrf_drv_clock.h"
#include "app_error.h"

#include "rtc.h"


#define DAY 86400UL  // 24 hours * 60 minutes * 60 seconds
#define BEGYEAR 2000 // UTC started at 00:00:00 January 1, 2000
#define is_leap_year(yr) (!((yr) % 400) || (((yr) % 100) && !((yr) % 4)))
#define year_length(yr) (is_leap_year(yr) ? 366 : 365)

volatile uint32_t gTimestamp = 1609459200 - 946684800;
//volatile uint32_t gTimestamp = 0;

#define RTC_TICKS (RTC_US_TO_TICKS(1000000ULL, RTC_DEFAULT_CONFIG_FREQUENCY))

#define RTC_CC 2

const nrf_drv_rtc_t rtc = NRF_DRV_RTC_INSTANCE(2); /**< Declaring an instance of nrf_drv_rtc for RTC2. */

void rtc_handler(nrf_drv_rtc_int_type_t int_type) {
    ret_code_t err_code;
    gTimestamp++;
    
    err_code = nrf_drv_rtc_cc_set(
        &rtc,
        RTC_CC,
        (nrf_rtc_cc_get(rtc.p_reg, RTC_CC) + RTC_TICKS) & RTC_COUNTER_COUNTER_Msk,
        true);
    APP_ERROR_CHECK(err_code);
}

void rtc_init(void) {
    ret_code_t err_code;

    //Initialize RTC instance
    nrf_drv_rtc_config_t config = NRF_DRV_RTC_DEFAULT_CONFIG;

    err_code = nrf_drv_rtc_init(&rtc, &config, rtc_handler);
    APP_ERROR_CHECK(err_code);

    err_code = nrf_drv_rtc_cc_set(&rtc, RTC_CC, RTC_TICKS, true);
    APP_ERROR_CHECK(err_code);

    //Power on RTC instance
    nrf_drv_rtc_enable(&rtc);
}

void rtc_set_time(uint32_t timestampNow) {
    gTimestamp = timestampNow;
}

uint32_t rtc_get_time(void) {
    return gTimestamp;
}

uint8_t month_length(uint8_t lpyr, uint8_t mon) {
    uint8_t days = 31;

    if (mon == 1) // feb 2
    {
        days = (28 + lpyr);
    }
    else
    {
        if (mon > 6) // aug-dec 8-12
        {
            mon--;
        }

        if (mon & 1)
        {
            days = 30;
        }
    }

    return (days);
}

uint8_t get_week(uint16_t year, uint8_t month, uint8_t day) {
    const uint8_t table_week[12] = {0, 3, 3, 6, 1, 4, 6, 2, 5, 0, 3, 5};
    uint16_t temp2;
    uint8_t yearH, yearL;
    uint8_t week;
    yearH = year / 100;
    yearL = year % 100;
    month = month % 13;

    if (yearH > 19)
        yearL += 100;

    temp2 = yearL + yearL / 4;

    temp2 = temp2 % 7;

    temp2 = temp2 + day + table_week[month - 1];

    if (yearL % 4 == 0 && month < 3)
        temp2--;

    if ((temp2 % 7) == 0)
    {
        temp2 = 7;
        week = temp2;
    }
    else
    {
        week = (temp2 % 7);
    }

    week--;
    return week;
}

void get_UTC_time(UTCTimeStruct *tm)
{
    uint32_t secTime = rtc_get_time();
    // calculate the time less than a day - hours, minutes, seconds
    {
        unsigned int day = secTime % DAY;
        tm->seconds = day % 60UL;
        tm->minutes = (day % 3600UL) / 60UL;
        tm->hour = day / 3600UL;
    }

    // Fill in the calendar - day, month, year
    {
        unsigned short numDays = secTime / DAY;
        tm->year = BEGYEAR;
        while (numDays >= year_length(tm->year))
        {
            numDays -= year_length(tm->year);
            tm->year++;
        }

        tm->month = 0;
        while (numDays >= month_length(is_leap_year(tm->year), tm->month))
        {
            numDays -= month_length(is_leap_year(tm->year), tm->month);
            tm->month++;
        }

        tm->day = numDays;
    }

    tm->day++;
    tm->month++;

    tm->week = get_week(tm->year, tm->month, tm->day);
}


const char *get_months_low( uint8_t month ) {

    char const *MonthsLow[] = {
        "January",
        "February",
        "March",
        "April",
        "May",
        "June",
        "July",
        "August",
        "September",
        "October",
        "November",
        "December"
    };

    return MonthsLow[month];

}

const char *get_days_low( uint8_t day ) {
    char const *DaysStringLow[] = {
        "Monday",
        "Tuesday",
        "Wednesday",
        "Thursday",
        "Friday",
        "Saturday",
        "Sunday"
    };
    return DaysStringLow[day];
}

const char *get_days_low_short( uint8_t day ) {
    char const *DaysStringLow[] = {
        "Mon",
        "Tue",
        "Wed",
        "Thu",
        "Fri",
        "Sat",
        "Sun"
    };
    return DaysStringLow[day];
}

const char *get_months( uint8_t month ) {
    char const *MonthsString[] = {
        "JAN",
        "FEB",
        "MAR",
        "APR",
        "MAY",
        "JUN",
        "JUL",
        "AUG",
        "SEP",
        "OCT",
        "NOV",
        "DEC"
    };
    return MonthsString[month];
}

const char *get_days( uint8_t day ) {
char const *DaysString[] = {        
        "MONDAY",
        "TUESDAY",
        "WEDNESDAY",
        "THURSDAY",
        "FRIDAY",
        "SATURDAY",
        "SUNDAY"
    };
    return DaysString[day];
}
