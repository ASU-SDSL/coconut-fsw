#pragma once

#include "pico/stdlib.h"

#include <string.h>

#include <FreeRTOS.h>
#include <task.h>
#include <semphr.h>
#include "queue.h"
#include "gse.h"
#include "ccsds.h"
#include "log.h"
#include "steve.h"

#define COMMAND_MAX_QUEUE_ITEMS 0x200
#define COMMAND_SYNC_BYTES "\x35\x2E\xF8\x53"

// Command Structs and Types
typedef enum command_apid {
    STORE_GROUNDNODE_DATA = 0,
    CHANGE_HEARTBEAT_TELEM_RATE = 1,
    REQUEST_DOWNLINK_GROUNDNODE_DATA = 2, 
    REQUEST_DOWNLINK_TELEMETRY_DATA = 3,
    LIST_STEVE_TASKS = 4
} command_apid_t;

typedef struct {
    uint32_t ms;
} change_heartbeat_telem_rate_t;

// Internal Command Thread Structs
QueueHandle_t command_byte_queue;
typedef char command_byte_t;

/* USER FUNCTIONS */
void receive_command_byte_from_isr(char ch);
void receive_command_byte(char ch);
void receive_command_bytes(uint8_t* packet, size_t packet_size);

/* INTERNAL FUNCTIONS */
void parse_command_packet(ccsds_header_t header, uint8_t* payload_buf, uint32_t payload_size);

// Main Task
void command_task(void* unused_arg);