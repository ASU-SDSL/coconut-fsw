#pragma once

#include <stdint.h>

#include "ccsds.h"
#include "user_auth.h"

#define COMMAND_MAX_QUEUE_ITEMS 0x200
#define COMMAND_SYNC_BYTES "\x35\x2E\xF8\x53"

// Command Structs and Types
typedef enum command_apid {
    UPLOAD_USER_DATA = 0,
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
    FILE_MKFS = 11,
    ADD_USER = 12,
    DELETE_USER = 13,
} command_apid_t;

typedef struct __attribute__((__packed__)) {
    uint8_t admin_token[TOKEN_LENGTH];
    uint32_t ms;
} change_heartbeat_telem_rate_t;

typedef struct __attribute__((__packed__)) {
    uint8_t admin_token[TOKEN_LENGTH];
    char path[0xFF];
} file_ls_t;

typedef struct __attribute__((__packed__)) {
    uint8_t admin_token[TOKEN_LENGTH];
    char path[0xFF];
} file_mkdir_t;

typedef struct __attribute__((__packed__)) {
    uint8_t admin_token[TOKEN_LENGTH];
    char path[0xFF];
} file_cat_t;

typedef struct __attribute__((__packed__)) {
    uint8_t admin_token[TOKEN_LENGTH];
    char path[0xFF];
} file_delete_t;

typedef struct __attribute__((__packed__)) {
    uint8_t admin_token[TOKEN_LENGTH];
    char path[0xFF];
    uint16_t data_len;
    uint8_t data[];
} file_append_t;

typedef struct __attribute__((__packed__)) {
    uint8_t admin_token[TOKEN_LENGTH];
    char path[0xFF];
} file_touch_t;

typedef struct __attribute__((__packed__)) {
    uint8_t admin_token[TOKEN_LENGTH];
    uint8_t confirm;
} file_mkfs_t;

typedef struct __attribute__((__packed__)) {
    uint8_t admin_token[TOKEN_LENGTH];
    char new_user_name[MAX_USERNAME_LEN + 1];
    uint8_t new_user_token[TOKEN_LENGTH];
} add_user_t;

typedef struct __attribute__((__packed__)) {
    uint8_t admin_token[TOKEN_LENGTH];
    char user_name[MAX_USERNAME_LEN + 1];
    uint8_t confirm;
} delete_user_t;

typedef struct __attribute__((__packed__)) {
    uint8_t user_token[TOKEN_LENGTH];
    uint16_t data_len;
    uint8_t data[];
} upload_user_data_t;

// Internal Command Thread Structs
QueueHandle_t command_byte_queue;
typedef uint8_t command_byte_t;

/* USER FUNCTIONS */
void receive_command_byte_from_isr(char ch);
void receive_command_byte(char ch);
void receive_command_bytes(uint8_t* packet, size_t packet_size);

/* INTERNAL FUNCTIONS */
void parse_command_packet(ccsds_header_t header, uint8_t* payload_buf, uint32_t payload_size);

// Main Task
void command_task(void* unused_arg);