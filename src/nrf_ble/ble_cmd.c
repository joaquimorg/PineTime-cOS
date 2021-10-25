#include <stdbool.h>
#include <stdint.h>
#include <stdlib.h>

#include "ble_cmd.h"
#include "sys.h"
#include "nrf_ble.h"
#include "app.h"
#include "rtc.h"
#include "motor.h"

static const char * notification_names_def[10] = {"Missed Call", "SMS", "Social", "e-Mail", "Calendar", "WhatsApp", "Messenger", "Instagram", "Twitter", "Skype"};

static uint32_t get_int(uint8_t pos) {    
    return inputBuffer[pos + 3] + (inputBuffer[pos + 2] << 8) + (inputBuffer[pos + 1] << 16) + (inputBuffer[pos] << 24);
}

static uint8_t get_byte(uint16_t pos) {
    return inputBuffer[pos];
}

static int get_string(void *str, uint16_t pos) {
    uint8_t size;
    char buff[64] = {};
    //char *b = (char *)str;

    size = get_byte( pos );
    
    str = (char*)malloc(size + 1);
    //memset(b, 0x00, size + 1);
    
    for (uint8_t i = 0; i < size; i++) {
        buff[i] = inputBuffer[(pos + 1) + i];
    }   
    buff[size + 1] = 0x00;
    str = (char *) &buff[0];
    
    return pos + 1 + size;
}

// usage:
//   float x, y, z;
//   int i = 0;
//   i += packFloat(&buffer[i], x);
//   i += packFloat(&buffer[i], y);
//   i += packFloat(&buffer[i], z);
int packFloat(void *buf, float x) {
    unsigned char *b = (unsigned char *)buf;
    unsigned char *p = (unsigned char *) &x;
    b[0] = p[3];
    b[1] = p[2];
    b[2] = p[1];
    b[3] = p[0];
    return 4;
}

int packInt(void *buf, int x) {
    unsigned char *b = (unsigned char *)buf;
    unsigned char *p = (unsigned char *) &x;
    b[0] = p[3];
    b[1] = p[2];
    b[2] = p[1];
    b[3] = p[0];
    return 4;
}

int packByte(void *buf, uint8_t x) {
    unsigned char *b = (unsigned char *)buf;
    unsigned char *p = (unsigned char *) &x;
    b[0] = p[0];
    return 1;
}
// ----------------------------------------------------------------------------------

static void get_notification(void) {
    uint8_t size;
    uint16_t i = 0;

    notification_t notification;

    notification.id = get_int(i);
    i += 4;

    size = get_byte( i++ );
    notification.subject = (char *)malloc(size);
    memcpy(notification.subject, (char *) &inputBuffer[i], size);
    i += size;

    size = get_byte( i++ );
    notification.body = (char *)malloc(size);
    memcpy(notification.body, (char *) &inputBuffer[i], size);
    i += size;
    
    notification.type = get_byte(i);
    notification.typeName = notification_names_def[notification.type];

    if ( pinetimecosBLE.notificationCount <= 4 ) {
        pinetimecosBLE.notification[pinetimecosBLE.notificationCount++] = notification;
    } else {        
        
        // free mem from oldest notification
        if ( pinetimecosBLE.notification[0].subject != NULL ) {
            free(pinetimecosBLE.notification[0].subject);
        }
        if (  pinetimecosBLE.notification[0].body != NULL ) {
            free( pinetimecosBLE.notification[0].body);
        }

        // delete oldest notification
        // move all notifications up
        for (uint8_t i = 0; i < pinetimecosBLE.notificationCount - 1; i++) {
            pinetimecosBLE.notification[i] = pinetimecosBLE.notification[i + 1];
        }

        pinetimecosBLE.notification[pinetimecosBLE.notificationCount - 1] = notification;
    }

    pinetimecosBLE.newNotification = true;
    
}

static void get_weather(void) {
    uint8_t size;
    uint16_t i = 0;

    pinetimecosBLE.weather.currentTemp = get_byte(i++);
    pinetimecosBLE.weather.currentHumidity = get_byte(i++);
    pinetimecosBLE.weather.todayMaxTemp = get_byte(i++);
    pinetimecosBLE.weather.todayMinTemp = get_byte(i++);

    if ( pinetimecosBLE.weather.location != NULL ) {
        free(pinetimecosBLE.weather.location);
    }
    size = get_byte( i++ );
    pinetimecosBLE.weather.location = (char *)malloc(size);
    memcpy(pinetimecosBLE.weather.location, (char *) &inputBuffer[i], size);

    if ( pinetimecosBLE.weather.currentCondition != NULL ) {
        free(pinetimecosBLE.weather.currentCondition);
    }
    i += size;
    size = get_byte( i++ );
    pinetimecosBLE.weather.currentCondition = (char *)malloc(size);
    memcpy(pinetimecosBLE.weather.currentCondition, (char *) &inputBuffer[i], size);
    
    pinetimecosBLE.weather.hasData = true;
    pinetimecosBLE.weather.newData = true;
}

void ble_command(uint8_t msg_type) {

    //pinetimecos.debug = msg_type;
    switch (msg_type) {
        case COMMAND_TIME_UPDATE:
            rtc_set_time(get_int(0));
            break;

        case COMMAND_NOTIFICATION:
            get_notification();
            app_push_message(NewNotification);
            break;

        case COMMAND_DELETE_NOTIFICATION:
            break;

        case COMMAND_SET_ALARMS:
            break;

        case COMMAND_SET_CALL:
            motor_start(10);
            break;

        case COMMAND_SET_MUSIC:
            break;

        case COMMAND_SET_MUSIC_INFO:
            break;

        case COMMAND_ACTIVITY_STATUS:
            break;

        case COMMAND_FIND_DEVICE:
            pinetimecosBLE.find_device = inputBuffer[0];
            motor_start(10);
            break;

        case COMMAND_VIBRATION:
            motor_start(10);
            break;

        case COMMAND_WEATHER:
            get_weather();
            break;

        default:
            break;
    }

    if (msg_type != COMMAND_BASE) {
        app_push_message(WakeUp);
    }
}


void ble_send_version(void) {
    uint8_t data[4] = {0x00, COMMAND_PT_VERSION, 0x00, 0x01};
    send_data_ble(data, 4);
}

void ble_send_battery(void) {

    uint8_t data[11] = {};
    uint8_t status = 0;

    if ( pinetimecos.batteryPercentRemaining == -1 ) {
        status = 0x01;
    } else if ( pinetimecos.chargingState == StatusON ) {
        status = 0x02;
    } else {
        status = 0x03;
    }

    uint8_t i = 0;
    data[i++] = 0x00;
    data[i++] = COMMAND_PT_BATTERY;

    i += packInt(&data[i], pinetimecos.batteryPercentRemaining);
    i += packFloat(&data[i], (float)pinetimecos.batteryVoltage / 1000);
    i += packByte(&data[i], status);


    send_data_ble(data, i);
}



void ble_connection(void) {
    pinetimecosBLE.weather.hasData = false;
    pinetimecosBLE.weather.newData = false;

    ble_send_version();
    ble_send_battery();
    xTimerStart(bleTimer, 0);
}


void ble_update(void) {
    ble_send_battery();
}