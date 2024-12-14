#pragma once

#include "pico/stdlib.h"

#include <string.h>
#include <stdint.h>
#include <stdbool.h>

#include <FreeRTOS.h>
#include <task.h>
#include <semphr.h>
#include "queue.h"
#include "gse.h"
#include "ccsds.h"
#include "log.h"
#include "steve.h"
#include "command.h"

#define COMMAND_MAX_QUEUE_ITEMS 0x200
#define COMMAND_SYNC_BYTES "\x35\x2E\xF8\x53"

#define PASSWORD_LENGTH 8U

// Command Structs and Types
typedef enum command_apid {
    STORE_GROUNDNODE_DATA = 0,
    CHANGE_HEARTBEAT_TELEM_RATE = 1,
    REQUEST_DOWNLINK_GROUNDNODE_DATA = 2, 
    REQUEST_DOWNLINK_TELEMETRY_DATA = 3,
    LIST_STEVE_TASKS = 4,
    FILE_LS = 5,
    FILE_MKDIR = 6,
    FILE_CAT = 7,
    FILE_DELETE = 8,
    FILE_APPEND = 9,
    FILE_TOUCH = 10,
    FILE_MKFS = 11
} command_apid_t;

typedef struct __attribute__((__packed__)) {
    char password[PASSWORD_LENGTH];
    uint32_t ms;
} change_heartbeat_telem_rate_t;

typedef struct __attribute__((__packed__)) {
    char password[PASSWORD_LENGTH];
    char path[0x100];
} file_ls_t;

typedef struct __attribute__((__packed__)) {
    char password[PASSWORD_LENGTH];
    char path[0x100];
} file_mkdir_t;

typedef struct __attribute__((__packed__)) {
    char password[PASSWORD_LENGTH];
    char path[0x100];
} file_cat_t;

typedef struct __attribute__((__packed__)) {
    char password[PASSWORD_LENGTH];
    char path[0x100];
} file_delete_t;

typedef struct __attribute__((__packed__)) {
    char password[PASSWORD_LENGTH];
    char path[0x100];
    uint16_t data_len;
    uint8_t data[];
} file_append_t;

typedef struct __attribute__((__packed__)) {
    char password[PASSWORD_LENGTH];
    char path[0x100];
} file_touch_t;

typedef struct __attribute__((__packed__)) {
    char password[PASSWORD_LENGTH];
    uint8_t confirm;
} file_mkfs_t;

// Internal Command Thread Structs
QueueHandle_t command_byte_queue;
typedef uint8_t command_byte_t;


/* USER FUNCTIONS */
void receive_command_byte_from_isr(char ch);
void receive_command_byte(char ch);
void receive_command_bytes(uint8_t* packet, size_t packet_size);

/* INTERNAL FUNCTIONS */
void parse_command_packet(ccsds_header_t header, uint8_t* payload_buf, uint32_t payload_size);
bool check_password(const char* password_buf);

// Main Task
void command_task(void* unused_arg);