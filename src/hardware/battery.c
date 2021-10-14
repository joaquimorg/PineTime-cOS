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

static nrf_saadc_value_t adc_buf;

/**
 * Symmetric sigmoidal approximation
 * https://www.desmos.com/calculator/7m9lu26vpy
 *
 * c - c / (1 + k*x/v)^3
 */
uint8_t sigmoidal(uint16_t voltage, uint16_t minVoltage, uint16_t maxVoltage) {
	// slow
	// uint8_t result = 110 - (110 / (1 + pow(1.468 * (voltage - minVoltage)/(maxVoltage - minVoltage), 6)));

	// steep
	// uint8_t result = 102 - (102 / (1 + pow(1.621 * (voltage - minVoltage)/(maxVoltage - minVoltage), 8.1)));

	// normal
	uint8_t result = 105 - (105 / (1 + pow(1.724 * (voltage - minVoltage)/(maxVoltage - minVoltage), 5.5)));
	return result >= 100 ? 100 : result;
}

void saadc_callback(nrf_drv_saadc_evt_t const * p_event) {

    const uint16_t battery_max = 4150; // maximum voltage of battery ( max charging voltage is 4.21 )
	const uint16_t battery_min = 3200; // minimum voltage of battery before shutdown ( depends on the battery )

    if (p_event->type == NRF_DRV_SAADC_EVT_DONE) {

        nrf_saadc_value_t adc_result;

        adc_result = p_event->data.done.p_buffer[0];

    
        // Voltage divider ratio
		//(R26 + R35) / R35
		uint16_t value = (VIN_MEAS_R26 + VIN_MEAS_R35) * adc_result / VIN_MEAS_R35;

        pinetimecos.batteryVoltage = ADC_RESULT_IN_MILLI_VOLTS(value);

        if (pinetimecos.batteryVoltage <= battery_min) {
			pinetimecos.batteryPercentRemaining = 0;
		} else if (pinetimecos.batteryVoltage >= battery_max) {
			pinetimecos.batteryPercentRemaining = 100;
		} else {
			pinetimecos.batteryPercentRemaining = sigmoidal(pinetimecos.batteryVoltage, battery_min, battery_max);
            if (pinetimecos.batteryVoltage <= 0) {
			    pinetimecos.batteryPercentRemaining = 0;
		    } else if (pinetimecos.batteryVoltage >= 100) {
			    pinetimecos.batteryPercentRemaining = 100;
		    }

		}

    }
}

void battery_init(void) {
   	ret_code_t err_code;
    nrf_saadc_channel_config_t channel_config =
        NRF_DRV_SAADC_DEFAULT_CHANNEL_CONFIG_SE(BATTERY_VOL);

    err_code = nrf_drv_saadc_init(NULL, saadc_callback);
    APP_ERROR_CHECK(err_code);

    err_code = nrf_drv_saadc_channel_init(0, &channel_config);
    APP_ERROR_CHECK(err_code);

    err_code = nrf_drv_saadc_buffer_convert(&adc_buf, 1);
    APP_ERROR_CHECK(err_code);

    err_code = nrf_drv_saadc_sample();
    APP_ERROR_CHECK(err_code);
}

void battery_read(void) {

    if (!nrf_drv_saadc_is_busy()) {
        ret_code_t err_code = nrf_drv_saadc_buffer_convert(&adc_buf, 1);
        APP_ERROR_CHECK(err_code);

        err_code = nrf_drv_saadc_sample();
        APP_ERROR_CHECK(err_code);
    }
}