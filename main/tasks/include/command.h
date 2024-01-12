#ifndef COMMAND_FILE_DEFINED
#define COMMAND_FILE_DEFINED

#include <stdio.h>
#include <string.h>

#include <FreeRTOS.h>
#include <task.h>
#include <semphr.h>
#include "pico/stdlib.h"
#include "queue.h"
#include "gse.h"
#include "ccsds.h"
#include "log.h"

#define COMMAND_MAX_QUEUE_ITEMS 64
#define COMMAND_CHECK_DELAY_MS 500
#define COMMAND_SYNC_BYTES "\x35\x2E\xF8\x53"
#define STARTING_COMMAND_BUF_SIZE 32

// Command Structs and Types
typedef enum command_apid {
    STORE_GROUNDNODE_DATA = 0,
    CHANGE_HEARTBEAT_TELEM_RATE = 1,
    REQUEST_DOWNLINK_GROUNDNODE_DATA = 2, 
    REQUEST_DOWNLINK_TELEMETRY_DATA = 3
} command_apid_t;

// Internal Command Thread Structs
QueueHandle_t command_byte_queue;
typedef char command_byte_t;

/* USER FUNCTIONS */
void receive_command_byte_from_isr(char ch);

/* INTERNAL FUNCTIONS */
void parse_command_packet(ccsds_header_t header, uint8_t* payload_buf, uint32_t payload_size);

// Main Task
void command_task(void* unused_arg);

#endif /* !COMMAND_FILE_DEFINED */