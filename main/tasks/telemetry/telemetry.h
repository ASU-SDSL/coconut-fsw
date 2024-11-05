#pragma once

#include <stdlib.h>
#include <string.h>

#include <FreeRTOS.h>
#include <task.h>
#include <semphr.h>

#include "gse.h"
#include "radio.h"
#include "state.h"

#define TELEMETRY_SYNC_BYTES "\x35\x2E\xF8\x53"
#define TELEMETRY_SYNC_SIZE 4U
#define TELEMETRY_MAX_QUEUE_ITEMS 128
#define TELEMETRY_CHECK_DELAY_MS portMAX_DELAY


uint16_t g_packet_sequence_number;

QueueHandle_t telemetry_queue;

/* TELEMETRY DEFINITIONS*/
typedef enum {
    LOG = 0,
    HEARTBEAT = 1, // just to keep the command and telemetry apid equal :)
    DOWNLINK_GROUNDNODE_DATA = 2, 
    DOWNLINK_TELEMETRY_DATA = 3
} telemetry_apid_t;

typedef struct __attribute__((__packed__)) {
    uint16_t size;
    char str[]; 
} log_telemetry_t;

typedef struct __attribute__((__packed__)) {
    payload_state_t state;
    TickType_t uptime;
    uint8_t hour;
    uint8_t minute;
    uint8_t second;
    uint8_t month;
    uint8_t date;
    uint8_t year;
    float rtcTemp; 
    uint16_t ina900_shunt;
    uint16_t ina900_vbus;
    uint16_t ina900_power;
    uint16_t ina900_current;
    uint16_t ina1000_shunt; 
    uint16_t ina1000_vbus;
    uint16_t ina1000_power;
    uint16_t ina1000_current;
    uint16_t ina1100_shunt; 
    uint16_t ina1100_vbus;
    uint16_t ina1100_power;
    uint16_t ina1100_current;
    uint16_t ina1200_shunt; 
    uint16_t ina1200_vbus;
    uint16_t ina1200_power;
    uint16_t ina1200_current;
    uint16_t ina1300_shunt; 
    uint16_t ina1300_vbus;
    uint16_t ina1300_power;
    uint16_t ina1300_current;
    int16_t mag_x;
    int16_t mag_y;
    int16_t mag_z;
    int16_t mag_temp;
    uint8_t vega_ant_status;
} heartbeat_telemetry_t;

/* USER FUNCTIONS */
void send_telemetry(telemetry_apid_t apid, const char* payload_buffer, size_t payload_size);

/* INTERNAL FUNCTIONS */
// Main Task
void telemetry_task(void* unused_arg);