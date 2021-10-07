#include "sys.h"

#include "lvgl.h"
#include "lvgl_init.h"
#include "nrf_ble.h"
#include "app.h"
#include "watchdog.h"
#include "rtc.h"

/* FreeRTOS related */
#include "FreeRTOS.h"
#include "task.h"
#include "semphr.h"


#define SYS_TASK_DELAY          1 
#define SYS_TASK_DELAY_SLEEP    100

TaskHandle_t  sys_task_handle;
TaskHandle_t  app_task_handle;



static void sys_task_function(void* pvParameter) {
    UNUSED_PARAMETER(pvParameter);

    init_watchdog();
    rtc_init();

    pinetimecos.appTask = xTaskCreate(main_app, "APP", configMINIMAL_STACK_SIZE + 600, NULL, 3, &app_task_handle);
    pinetimecos.state = Running;

    while (true)
    {
        if(pinetimecos.state == Running) {
            vTaskDelay(SYS_TASK_DELAY);
            lv_tick_inc(SYS_TASK_DELAY);
            lv_timer_handler();
        } else {
            vTaskDelay(SYS_TASK_DELAY_SLEEP);
        }
        
        feed_watchdog();
        /* Tasks must be implemented to never return... */
    }
}


void sys_init(void) {
    
    lvgl_init();

    UNUSED_VARIABLE(xTaskCreate(sys_task_function, "SYS", configMINIMAL_STACK_SIZE + 200, NULL, 3, &sys_task_handle));

    nrf_ble_init();

}