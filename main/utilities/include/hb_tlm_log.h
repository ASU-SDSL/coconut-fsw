#pragma once
#include "telemetry.h"
#include <stdint.h>

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
void log_heartbeat_tlm(heartbeat_telemetry_t payload);

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
*  - Returns 0 on success, 1 on missing tlm directory, 2 on f_info.bin read error, 5 on reached end of logs
*/
int hb_tlm_playback(playback_hb_tlm_payload_t playback_payload);
