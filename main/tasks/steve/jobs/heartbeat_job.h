#pragma once

#include <stdint.h>

#include "state.h"
#include "telemetry.h"
#include "eps.h"
#include "mag.h"
#include "rtc.h"

#define HEARTBEAT_JOB_NAME "heartbeat_telemetry"
#define HEARTBEAT_TELEMETRY_DEFAULT_INTERVAL 30

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
    uint16_t eps_shunt;
    uint16_t eps_vbus;
    uint16_t eps_power;
    uint16_t eps_current;
    int16_t mag_x;
    int16_t mag_y;
    int16_t mag_z;
    int16_t mag_temp;
} heartbeat_telemetry_t;

uint32_t iteration_counter;

void heartbeat_telemetry_job(void* unused);