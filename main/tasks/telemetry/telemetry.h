#pragma once

#include "pico/stdlib.h"

#include <stdio.h>
#include <string.h>

#include <FreeRTOS.h>
#include <task.h>
#include <semphr.h>


#include "gse.h"
#ifndef SIMULATOR
#include "radio.h"
#endif

#include "state.h"

#define TELEMETRY_SYNC_BYTES "\x35\x2E\xF8\x53"
#define TELEMETRY_SYNC_SIZE 4U
#define TELEMETRY_MAX_QUEUE_ITEMS 64
#define TELEMETRY_CHECK_DELAY_MS 500

uint16_t g_packet_sequence_number;

QueueHandle_t telemetry_queue;

/* TELEMETRY DEFINITIONS*/
typedef enum {
    RESERVED = 0,
    HEARTBEAT = 1, // just to keep the command and telemetry apid equal :)
    DOWNLINK_GROUNDNODE_DATA = 2, 
    DOWNLINK_TELEMETRY_DATA = 3
} telemetry_apid_t;

/* USER FUNCTIONS */
void send_telemetry(telemetry_apid_t apid, const char* payload_buffer, size_t payload_size);

/* INTERNAL FUNCTIONS */
// Main Task
void telemetry_task(void* unused_arg);