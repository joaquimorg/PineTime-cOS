#include "motor.h"
#include "sys.h"
#include "utils.h"

void motor_stop(void) {
    pinetimecos.motorState = StatusOFF;
    BaseType_t xHigherPriorityTaskWoken = pdFALSE;
    nrf_gpio_pin_set(VIBRATOR_CTRL);
    if (in_isr()) {
        xTimerStartFromISR(pinetimecos.motor_timer, &xHigherPriorityTaskWoken);
    } else {
        xTimerStop(pinetimecos.motor_timer, 0);
    }
}

static void motor_timer_callback(TimerHandle_t xTimer) {
    motor_stop();
}

void motor_start(uint8_t durationMs) {
    BaseType_t xHigherPriorityTaskWoken = pdFALSE;

    if ( pinetimecos.motorState == StatusON ) return;
    pinetimecos.motorState = StatusON;
    if (in_isr()) {
        xTimerChangePeriodFromISR( pinetimecos.motor_timer, pdMS_TO_TICKS( durationMs ), &xHigherPriorityTaskWoken );
        xTimerStartFromISR(pinetimecos.motor_timer, &xHigherPriorityTaskWoken);
    } else {
        xTimerChangePeriod( pinetimecos.motor_timer, pdMS_TO_TICKS( durationMs ), 0 );
        xTimerStart(pinetimecos.motor_timer, 0);
    }        
    nrf_gpio_pin_clear(VIBRATOR_CTRL);
}

void motor_init(void){
    pinetimecos.motorState = StatusOFF;
    nrf_gpio_cfg_output(VIBRATOR_CTRL);
    nrf_gpio_pin_set(VIBRATOR_CTRL);
    pinetimecos.motor_timer = xTimerCreate("MOT", pdMS_TO_TICKS( 0 ), pdFALSE, NULL, motor_timer_callback);
}

