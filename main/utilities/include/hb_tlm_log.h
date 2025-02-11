#pragma once
#include <stdint.h>
//#include "telemetry.h"

typedef struct __attribute__((__packed__)) {
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

    int16_t rfm_state; 
    int16_t sx_state; 
    uint8_t which_radio; // 1 for RFM
} heartbeat_telemetry2_t;



// Contents of f_info.bin file which keeps track of information about the logging size and files for the heartbeat tlm in the /tlm directory
typedef struct HBTlmFilesInfo {
    uint16_t file_index; // Keeps track of the most recent index of tlm file (not necessarily how many files are in the current directory)
    uint16_t oldest_file_index; // Index or name of the oldest tlm log in the directory
    uint16_t directory_size; // Keeps track of the total size that these files are taking up
} hb_tlm_file_size_info_t;

/* 
*  Keep a /tlm directory where:
*  - /tlm/file_counter.bin file contains the index of the next file to be created - 2 bytes
*  - /tlm/0.bin, /tlm/1.bin, ... /tlm/65535 files contain the telemetry data, most recent of which is the number.bin contained in the file_counter.txt file
*/
void log_heartbeat_tlm(heartbeat_telemetry2_t payload);

// Command payload for the playback heartbeat telemetry command
typedef struct PlaybackHBTLMPayload {
    uint16_t number_of_packets;
    uint16_t every_x_packet; // Used to adjust for less resolution but cover more time
    uint16_t go_back_x_packets; // Used to start the playback from a certain point in the past
} playback_hb_tlm_payload_t;

/*
*  Command for playing back the heartbeat telemetry logs
*  - Goes through each file, aquiring packets until number_of_packets at intervals of every_x_packet
*  - If go_back_x_packets is 0, it will begin at the most recently logged packet, otherwise it will traverse back go_back_x_packets packets first before collecting packets
*  - Returns 0 on success, 1 on missing tlm directory, 2 on f_info.bin read error
*/
int hb_tlm_playback(playback_hb_tlm_payload_t playback_payload);
