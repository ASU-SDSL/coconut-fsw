#include "hb_tlm_log.h"
#include "log.h"
#include "filesystem.h"
#include <stdbool.h>
#include <stdio.h>

// Size (bytes) that a heartbeat telemetry file can be until a new one should be used
// If writing a new telemetry entry causes the file to go over this amount, a new file is made
#define MAX_HB_TLM_FILE_SIZE 2000 // About an 15 minute of tlm, small enough to read into memory for playback

// Tlm files of the size specified by MAX_HB_TLM_FILE_SIZE will continue to be created until creating a new file of said size will surpase or equal this limit
// Once this limit is reached, the oldest log will be deleted before a new one is created
#define MAX_HB_TLM_TOTAL_SIZE 10000 //1250000 // 10 MBit to bytes

void log_heartbeat_tlm(heartbeat_telemetry2_t payload) {

    // See if tlm directory exists
    if (!dir_exists("/tlm")) {
        make_dir("/tlm");

        // Create the file info file as well

        hb_tlm_file_size_info_t f_data = {
            0, // Start at index 0
            0, // Oldest file has index 0
            sizeof(hb_tlm_file_size_info_t) // directory only has this file in it, so start the directory size at this
        };
        write_file("/tlm/f_info.bin", (char*)&f_data, sizeof(hb_tlm_file_size_info_t), false); // create and write to the file info file

        logln_info("TLM dir:");
        list_dir("/tlm");
    }

    // Read the file info
    hb_tlm_file_size_info_t file_info_buf;
    int bytes_read = read_file("/tlm/f_info.bin", (char*)&file_info_buf, sizeof(hb_tlm_file_size_info_t));
    if (bytes_read != sizeof(hb_tlm_file_size_info_t)) {
        logln_error("Error reading f_info.bin");
        return;
    }

    logln_info("Heartbeat tlm file index: %d", file_info_buf.file_index);
    
    // Write to this indexed file
    char filename[20];
    snprintf(filename, sizeof(filename), "/tlm/%d.bin", file_info_buf.file_index);
    logln_info("Writing to file: %s", filename);
    write_file(filename, (char*)&payload, sizeof(payload), true); // append - this will eaither make the file if it does not exist or append to it if it does

    // Check file size to see if it is time to make a new file. If writing another hb to the file makes it go over the limit, 
    FILINFO file_info;
    stat(filename, &file_info);
    if (file_info.fsize + sizeof(heartbeat_telemetry2_t) > MAX_HB_TLM_FILE_SIZE) {
        // Increase f_count.bin by 1 for next time
        file_info_buf.file_index += 1;
        file_info_buf.directory_size += MAX_HB_TLM_FILE_SIZE; // Only increase once the file is full, it doesn't get checked until the file is full anyway

        // See if there is enough space for the new file, otherwise delete the oldest file
        if (file_info_buf.directory_size + MAX_HB_TLM_FILE_SIZE > MAX_HB_TLM_TOTAL_SIZE) {
            // Delete the oldest file in the directory an increase the oldest file counter by 1
            char oldest_filename[20];
            snprintf(oldest_filename, sizeof(oldest_filename), "/tlm/%d.bin", file_info_buf.oldest_file_index);
            delete_file(oldest_filename);
            logln_info("Deleted oldest file: %s", oldest_filename);
            file_info_buf.oldest_file_index += 1;

            file_info_buf.directory_size -= MAX_HB_TLM_FILE_SIZE; // Remove the size of that full file, this value will end up oscillating once the directory is full
        }
    }

    logln_info("Tlm file info: %d, %d, %d", file_info_buf.file_index, file_info_buf.oldest_file_index, file_info_buf.directory_size);

    // Write the new file info information in case anything changed
    write_file("/tlm/f_info.bin", (char*)&file_info_buf, sizeof(hb_tlm_file_size_info_t), false);

    logln_info("\nTLM DIR:");
    list_dir("/tlm");
    
}

int hb_tlm_playback(playback_hb_tlm_payload_t playback_payload) {
    
    logln_info("PLAYBACK");

    // Make sure tlm exists
    if (!dir_exists("/tlm")) {
        logln_error("No tlm directory found on playback");
        return 1;
    }

    // Read the file info
    hb_tlm_file_size_info_t file_info_buf;
    int bytes_read = read_file("/tlm/f_info.bin", (char*)&file_info_buf, sizeof(hb_tlm_file_size_info_t));
    if (bytes_read != sizeof(hb_tlm_file_size_info_t)) {
        logln_error("Error reading f_info.bin");
        return 2;
    }

    // Start at the most recent file
    uint16_t file_index = file_info_buf.file_index;
    char current_log_filename[20];
    snprintf(current_log_filename, sizeof(current_log_filename), "/tlm/%d.bin", file_index);

    int packet_counter = 0; // keep track of how many packets have been transmitted until file_info_buf.number_of_packets
    while (packet_counter < playback_payload.number_of_packets) {

        // For now, read the entire file into memory, this may need to be changed, or maybe just the MAX_HB_TLM_FILE_SIZE should be changed
        char tlm_buf[MAX_HB_TLM_FILE_SIZE];
        bytes_read = read_file(current_log_filename, tlm_buf, MAX_HB_TLM_FILE_SIZE);
        bool partial_read = false;
        if (bytes_read != MAX_HB_TLM_FILE_SIZE) {
            if (bytes_read > 0) {
                // End of logs - only read a partial file
                partial_read = true;
            } else {
                logln_error("Playback - read error occured");
                return 3;
            }
        }

        // Overwrite with the next file index for the next loop if needed
        file_index -= 1;
        if (file_index < file_info_buf.oldest_file_index) {
            logln_error("Playback - reached end of logs");
            return 4;
        }
        snprintf(current_log_filename, sizeof(current_log_filename), "/tlm/%d.bin", file_info_buf.file_index);

        // Loop through this specific file
        // file_index is where the next heartbeat packet starts in the file buffer - going backwards
        int packet_index = bytes_read - sizeof(heartbeat_telemetry2_t);
        heartbeat_telemetry2_t hb_tlm_buf;
        while (packet_index >= 0) {
            // Check if this packet should be sent
            if (packet_counter % playback_payload.every_x_packet != 0) {
                continue;
            }

            // If we want to send this one, read it into memory and send it
            memcpy(&hb_tlm_buf, packet_index, sizeof(heartbeat_telemetry2_t));

            logln_info("Sending playback packet #%d, from file %s, with index into file of %d", packet_counter, current_log_filename, packet_index);

            // Send the packet
            send_telemetry(7/*HEARTBEAT_PLAYBACK*/, (char*)&hb_tlm_buf, sizeof(heartbeat_telemetry2_t));
            packet_counter += 1;

            // Check if we have reached the end
            if (packet_counter >= playback_payload.number_of_packets) {
                break;
            }

            // Move to the next packet
            packet_index -= sizeof(heartbeat_telemetry2_t);
        }
    }

    return 0;

}
