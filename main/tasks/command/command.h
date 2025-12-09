/**
 * @file command.h
 * @brief Handles decoding and triggering responses to received commands 
 * 
 */

#pragma once

#include <stdint.h>
#include <FreeRTOS.h>

#include "spacepacket.h"
#include "user_auth.h"

#define COMMAND_MAX_QUEUE_ITEMS 0x200
#define COMMAND_SYNC_BYTES "\x35\x2E\xF8\x53" 
#define AX25_FLAG 0x7E

extern TaskHandle_t xCommandTaskHandler;

/**
 * @brief Command Structs and Types
 * Groups: 
 * 00 - general and file system 
 * 01 - radio
 * 02 - device 
 */
typedef enum command_apid {
    // 00 - general and file system 
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
    MCU_POWER_CYCLE = 14,
    PLAYBACK_HEARTBEAT_PACKETS = 15,
    FSW_ACK = 16,
    SYS_INFO = 17,
    APID_INITIALIZE_FILE_DOWNLINK = 17,
    APID_FILE_DOWNLINK_ACK = 18,
    APID_FILE_DOWNLINK_CHANGE_PACKET_SIZE = 19,

    // 01 - radio
    RADIO_CONFIG = 101,
    RADIO_ECHO = 102, // not used
    RADIO_SET_MODE = 103,
    RADIO_STAT = 104, 
    ANTENNA_DEPLOY = 105,
    AX25_ON_OFF = 106,

    // 02 - device 
    SET_RTC_TIME = 201, 
} command_apid_t;

// Command type definitions 

typedef struct __attribute__((__packed__)) {
    uint8_t admin_token[TOKEN_LENGTH];
    uint32_t ms;
} change_heartbeat_telem_rate_t;

typedef struct __attribute__((__packed__)) {
    uint8_t admin_token[TOKEN_LENGTH];
    char path[MAX_PATH_SIZE];
} file_ls_t;

typedef struct __attribute__((__packed__)) {
    uint8_t admin_token[TOKEN_LENGTH];
    char path[MAX_PATH_SIZE];
} file_mkdir_t;

typedef struct __attribute__((__packed__)) {
    uint8_t admin_token[TOKEN_LENGTH];
    char path[MAX_PATH_SIZE];
} file_cat_t;

typedef struct __attribute__((__packed__)) {
    uint8_t admin_token[TOKEN_LENGTH];
    char path[MAX_PATH_SIZE];
} file_delete_t;

typedef struct __attribute__((__packed__)) {
    uint8_t admin_token[TOKEN_LENGTH];
    char path[MAX_PATH_SIZE];
    uint16_t data_len;
    uint8_t data[];
} file_append_t;

typedef struct __attribute__((__packed__)) {
    uint8_t admin_token[TOKEN_LENGTH];
    char path[MAX_PATH_SIZE];
} file_touch_t;

typedef struct __attribute__((__packed__)) {
    uint8_t admin_token[TOKEN_LENGTH];
    uint8_t confirm;
} file_mkfs_t;

typedef struct __attribute__((__packed__)) {
    uint8_t admin_token[TOKEN_LENGTH];
    uint8_t new_user_token[TOKEN_LENGTH];
    char new_user_name[MAX_USERNAME_LEN + 1];
} add_user_t;

typedef struct __attribute__((__packed__)) {
    uint8_t admin_token[TOKEN_LENGTH];
    char user_name[MAX_USERNAME_LEN + 1];
    uint8_t confirm;
} delete_user_t;

typedef struct __attribute__((__packed__)) {
    uint8_t user_token[TOKEN_LENGTH];
    uint8_t data_len;
    uint8_t data[];
} upload_user_data_t;

typedef struct __attribute__((__packed__)) {
    uint8_t admin_token[TOKEN_LENGTH];
} mcu_power_cycle_t; 

typedef struct __attribute__((__packed__)) {
    uint8_t admin_token[TOKEN_LENGTH];
    uint16_t number_of_packets;
    uint16_t every_x_packet; ///< Used to adjust for less resolution but cover more time
    uint16_t go_back_x_packets; ///< Used to start the playback from a certain point in the past
} playback_hb_tlm_payload_t;

// Note: Initialize file downlink command only has a string as its payload

typedef struct __attribute__((__packed__)) {
    uint8_t transaction_id; // Specific ID to ensure ground and satellite continue to transfer the same file
    uint16_t sequence_number; // Specifically for the file downlink protocol, NOT the same as CCSDS space packet
} file_downlink_ack_payload_t;

typedef struct __attribute__((__packed__)) {
    uint8_t new_packet_size;
} file_downlink_change_packet_size_payload_t;

typedef struct __attribute__((__packed__)) {
    uint8_t admin_token[TOKEN_LENGTH];
    uint8_t selected_radio; ///< radio to switch to (1 == RFM, 0 == SX, anything else no change)
    uint8_t updated_power; ///< output power to set (0 == no change)
} radio_config_t; 

typedef struct __attribute__((__packed__)) { 
    uint8_t admin_token[TOKEN_LENGTH];
} radio_stat_t; 

typedef struct __attribute__((__packed__)) {
    uint8_t admin_token[TOKEN_LENGTH];
    uint8_t year;
    uint8_t month;
    uint8_t day;
    uint8_t hour; 
    uint8_t minute;
    uint8_t second;
} set_rtc_time_t; 

typedef struct __attribute__((__packed__)) {
    uint8_t admin_token[TOKEN_LENGTH];
    uint8_t radio_mode; 
} radio_set_mode_t; 

typedef struct __attribute__((__packed__)) { 
    uint8_t admin_token[TOKEN_LENGTH];
} sys_info_t; 

typedef struct __attribute__((__packed__)) { 
    uint8_t admin_token[TOKEN_LENGTH];
} ax25_on_off_t; 

/// Internal Command Thread Structs
QueueHandle_t command_byte_queue;
typedef uint8_t command_byte_t;


/* USER FUNCTIONS */
void receive_command_byte_from_isr(char ch);
void receive_command_byte(char ch);
void receive_command_bytes(uint8_t* packet, size_t packet_size);
uint32_t get_command_count(void);
/* INTERNAL FUNCTIONS */
void parse_command_packet(spacepacket_header_t header, uint8_t* payload_buf, uint32_t payload_size);

// Main Task
void command_task(void* unused_arg);
