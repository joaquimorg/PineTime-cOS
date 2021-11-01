#include <stdint.h>
#include <stdio.h>
#include <math.h>

#include "battery.h"
#include "pinetime_board.h"
#include "sys.h"

#include "nrf.h"
#include "nrf_drv_saadc.h"
#include "app_error.h"
#include "nrf_delay.h"
#include "nrf_log.h"
#include "nrf_log_ctrl.h"
#include "nrf_log_default_backends.h"


#define ADC_REF_VOLTAGE_IN_MILLIVOLTS  600  //!< Reference voltage (in milli volts) used by ADC while doing conversion.
#define ADC_RES_10BIT                  1024 //!< Maximum digital value for 10-bit ADC conversion.
#define ADC_PRE_SCALING_COMPENSATION   6    //!< The ADC is configured to use VDD with 1/3 prescaling as input. And hence the result of conversion is to be multiplied by 3 to get the actual value of the battery voltage.
#define ADC_RESULT_IN_MILLI_VOLTS(ADC_VALUE) \
    ((((ADC_VALUE) * ADC_REF_VOLTAGE_IN_MILLIVOLTS) / ADC_RES_10BIT) * ADC_PRE_SCALING_COMPENSATION)

#define VIN_MEAS_R26 1000 + 50  // 1000kOhm +- 5% + error correction
#define VIN_MEAS_R35 1000       // 1000kOhm +- 5%

#define BATT_BUFFER_SIZE 5
nrf_saadc_value_t batt_buffer[BATT_BUFFER_SIZE];
uint8_t batt_buffer_index = 0;

#define VOLTAGE_DIVISIONS 21 
static const float generic_lipo[VOLTAGE_DIVISIONS] = 
    { 3.27, 3.61, 3.69, 3.71, 3.73, 3.75, 3.77, 3.79, 3.8, 3.82, 3.84, 3.85, 3.87, 3.91, 3.95, 3.98, 4.02, 4.08, 4.11, 4.15, 4.2 }; // Voltage


static int charge_at_index (int i) {
    return i*5;
}

static int voltage_percentage(float battery_voltage) {
    int i = 0;

    if (generic_lipo[0] > battery_voltage) return 0; // voltage below charts

    while (i < VOLTAGE_DIVISIONS) {
      if (generic_lipo[i] < battery_voltage) i++;
      else {
        // Scanner found the correct
        float m = (charge_at_index(i) - charge_at_index(i-1))/(generic_lipo[i] - generic_lipo[i-1]);
        float c = charge_at_index(i) - (m * generic_lipo[i]);

        return (int)(battery_voltage * m + c);
      }
    }

    return 100; // Voltage over chart
}


void analogCalibrateOffset( void ) {
    // Enable the SAADC
    NRF_SAADC->ENABLE = 0x01;

    // Be sure the done flag is cleared, then trigger offset calibration
    NRF_SAADC->EVENTS_CALIBRATEDONE = 0x00;
    NRF_SAADC->TASKS_CALIBRATEOFFSET = 0x01;

    // Wait for completion
    while (!NRF_SAADC->EVENTS_CALIBRATEDONE);

    // Clear the done flag  (really shouldn't have to do this both times)
    NRF_SAADC->EVENTS_CALIBRATEDONE = 0x00;

    // Disable the SAADC
    NRF_SAADC->ENABLE = 0x00;

}

void battery_init(void) {
    analogCalibrateOffset();
}


void battery_read(void) {

    volatile int16_t adc_result = 0;

    NRF_SAADC->RESOLUTION = SAADC_RESOLUTION_VAL_10bit;

    NRF_SAADC->ENABLE = (SAADC_ENABLE_ENABLE_Enabled << SAADC_ENABLE_ENABLE_Pos);
    for (int i = 0; i < 8; i++) {
        NRF_SAADC->CH[i].PSELN = SAADC_CH_PSELP_PSELP_NC;
        NRF_SAADC->CH[i].PSELP = SAADC_CH_PSELP_PSELP_NC;
    }
    NRF_SAADC->CH[0].CONFIG = ((SAADC_CH_CONFIG_RESP_Bypass         << SAADC_CH_CONFIG_RESP_Pos)   & SAADC_CH_CONFIG_RESP_Msk)
                                | ((SAADC_CH_CONFIG_RESP_Bypass     << SAADC_CH_CONFIG_RESN_Pos)   & SAADC_CH_CONFIG_RESN_Msk)
                                | ((SAADC_CH_CONFIG_GAIN_Gain1_6    << SAADC_CH_CONFIG_GAIN_Pos)   & SAADC_CH_CONFIG_GAIN_Msk)
                                | ((SAADC_CH_CONFIG_REFSEL_Internal << SAADC_CH_CONFIG_REFSEL_Pos) & SAADC_CH_CONFIG_REFSEL_Msk)
                                | ((SAADC_CH_CONFIG_TACQ_3us        << SAADC_CH_CONFIG_TACQ_Pos)   & SAADC_CH_CONFIG_TACQ_Msk)
                                | ((SAADC_CH_CONFIG_MODE_SE         << SAADC_CH_CONFIG_MODE_Pos)   & SAADC_CH_CONFIG_MODE_Msk)
                                | ((SAADC_CH_CONFIG_BURST_Disabled  << SAADC_CH_CONFIG_BURST_Pos)  & SAADC_CH_CONFIG_BURST_Msk);
    NRF_SAADC->CH[0].PSELN = SAADC_CH_PSELP_PSELP_AnalogInput7;
    NRF_SAADC->CH[0].PSELP = SAADC_CH_PSELP_PSELP_AnalogInput7;

    NRF_SAADC->RESULT.PTR = (uint32_t)&adc_result;
    NRF_SAADC->RESULT.MAXCNT = 1; // One sample

    NRF_SAADC->TASKS_START = 0x01UL;

    while (!NRF_SAADC->EVENTS_STARTED);
    NRF_SAADC->EVENTS_STARTED = 0x00UL;

    NRF_SAADC->TASKS_SAMPLE = 0x01UL;

    while (!NRF_SAADC->EVENTS_END);
    NRF_SAADC->EVENTS_END = 0x00UL;

    NRF_SAADC->TASKS_STOP = 0x01UL;

    while (!NRF_SAADC->EVENTS_STOPPED);
    NRF_SAADC->EVENTS_STOPPED = 0x00UL;

    NRF_SAADC->ENABLE = (SAADC_ENABLE_ENABLE_Disabled << SAADC_ENABLE_ENABLE_Pos);
    
    if (adc_result < 0) {
        adc_result = 0;
    }

    batt_buffer[batt_buffer_index++] = adc_result;
    if (batt_buffer_index >= BATT_BUFFER_SIZE) {
        batt_buffer_index = 0;
    }
    adc_result = 0;
    for(uint8_t i = 0; i < BATT_BUFFER_SIZE; i++) {
        adc_result += batt_buffer[i];
    }
    adc_result /= BATT_BUFFER_SIZE;

    // Voltage divider ratio
    //(R26 + R35) / R35
    uint16_t value = (VIN_MEAS_R26 + VIN_MEAS_R35) * adc_result / VIN_MEAS_R35;

    pinetimecos.batteryVoltage = ADC_RESULT_IN_MILLI_VOLTS(value);

    pinetimecos.batteryPercentRemaining = voltage_percentage((float)pinetimecos.batteryVoltage / 1000);

    NRF_SAADC->INTENCLR = (SAADC_INTENCLR_END_Clear << SAADC_INTENCLR_END_Pos);
	NVIC_ClearPendingIRQ(SAADC_IRQn);
}

char * battery_get_icon(void) {
    if (pinetimecos.batteryPercentRemaining == -1) return "\xEE\xA4\x87";
    if (pinetimecos.batteryPercentRemaining > 80) return "\xEE\xA4\xA0";
    if (pinetimecos.batteryPercentRemaining > 60) return "\xEE\xA4\xA1";
    if (pinetimecos.batteryPercentRemaining > 40) return "\xEE\xA4\xA2";
    if (pinetimecos.batteryPercentRemaining > 20) return "\xEE\xA4\xA3";
    return "\xEE\xA4\xA4";
}