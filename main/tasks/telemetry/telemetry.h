/**
 * @file telemetry.h
 * @brief Functions for packaging and queuing telemetry packets for sending 
 * 
 */
#pragma once

#include "FreeRTOS.h" // This is needed because it gets included from "hb_tlm_log.h" before anything with an '#include "FreeRTOS.h"' is included for some reason
#include "queue.h"
#include "file_downlink.h"

#define TELEMETRY_SYNC_BYTES "\x35\x2E\xF8\x53"
#define TELEMETRY_SYNC_SIZE 4U
#define TELEMETRY_MAX_QUEUE_ITEMS 128
#define TELEMETRY_CHECK_DELAY_MS portMAX_DELAY

uint16_t g_packet_sequence_number;

QueueHandle_t telemetry_queue;

/* TELEMETRY DEFINITIONS*/
/**
 * @brief Telemetry packet APIDs
 * Groups: 
 * 00 - general and file system 
 * 01 - radio
 * 02 - device
 */
typedef enum {
    // 00 - general and file system 
    LOG_APID = 0,
    HEARTBEAT_APID = 1, // just to keep the command and telemetry apid equal :)
    FILE_DOWNLINK_APID = 2, 
    DOWNLINK_TELEMETRY_DATA_APID = 3,
    HEARTBEAT_PLAYBACK_APID = 4,
    ACK_APID = 5,
    FS_LOG_APID = 6, 

    // 01 - radio
    RADIO_STAT_RES = 101,
    
    // 02 - device 
} telemetry_apid_t;

/// @brief Log packet definition (sent over hardline only by default)
typedef struct __attribute__((__packed__)) {
    uint16_t size;
    char str[]; 
} log_telemetry_t;

/// @brief Heartbeat packet definition 
typedef struct __attribute__((__packed__)) {
    char callsign[8];
    uint8_t state;
    uint32_t uptime;
    uint8_t hour;
    uint8_t minute;
    uint8_t second;
    uint8_t month;
    uint8_t date;
    uint8_t year;
    float rtcTemp; 
    uint16_t ina0_shunt;
    uint16_t ina0_vbus;
    uint16_t ina0_power;
    uint16_t ina0_current;
    uint16_t ina1_shunt; 
    uint16_t ina1_vbus;
    uint16_t ina1_power;
    uint16_t ina1_current;
    uint16_t ina2_shunt; 
    uint16_t ina2_vbus;
    uint16_t ina2_power;
    uint16_t ina2_current;
    uint16_t ina3_shunt; 
    uint16_t ina3_vbus;
    uint16_t ina3_power;
    uint16_t ina3_current;
    uint16_t ina4_shunt; 
    uint16_t ina4_vbus;
    uint16_t ina4_power;
    uint16_t ina4_current;
    uint16_t ina5_shunt; 
    uint16_t ina5_vbus;
    uint16_t ina5_power;
    uint16_t ina5_current;
    float max17048Voltage;
    float max17048Percentage;
    int16_t mag_x;
    int16_t mag_y;
    int16_t mag_z;
    int16_t mag_temp;
    uint8_t vega_ant_status;
   
    int16_t temp_eps;
    int16_t temp_Xm;
    int16_t temp_Xp;
    int16_t temp_Ym;
    int16_t temp_Yp;
    int16_t temp_Zm;
    int16_t temp_Zp;

    int16_t rfm_state;
    int16_t sx_state; 
    
    uint8_t which_radio; ///< 1 for RFM
    uint32_t command_count;
    uint32_t boot_count; 
} heartbeat_telemetry_t;

/// @brief Telemetry response to any commands sent to the satellite
typedef struct __attribute__((__packed__)) {
    uint8_t command_status; ///< 1 for success/true, 0 for failure/false
    char last_logged_error[24];
    uint8_t *data; ///< Any extra data that might be returned by a command
} ack_telemetry_t;

// File downlink from file-downlink task
#define FILE_DOWNLINK_PATH_NAME_CHARS 24
typedef struct __attribute__((__packed__)) {
    bool eof; // If this packet is the end of the file - the ground should ack if this is true
    uint8_t transaction_id;
    uint16_t sequence_number;
    char path_name[FILE_DOWNLINK_PATH_NAME_CHARS]; // Limit this buffer so the radio packet doesn't get too big
    uint8_t data[MAX_DOWNLINK_PACKET_SIZE]; // File data - the max this can be is 255 byte chunks of downlinked data at a time (length is only 1 byte)
} file_downlink_telemetry_t;

typedef struct __attribute__((__packed__)) {
    float rssi;
    float snr;
    float frequency_error;
} radio_stat_telemetry_t;

/* USER FUNCTIONS */
void send_telemetry(telemetry_apid_t apid, const char* payload_buffer, size_t payload_size);

/* INTERNAL FUNCTIONS */
// Main Task
void telemetry_task(void* unused_arg);
