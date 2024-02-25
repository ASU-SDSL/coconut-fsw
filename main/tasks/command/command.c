#include "command.h"

void receive_command_byte_from_isr(char ch) {
    // ONLY USE FROM INTERRUPTS, CREATE NEW METHOD FOR QUEUEING CMD BYTES FROM TASKS
    // // Build command_char struct
    // command_byte_t command_byte;
    // command_byte.value = ch;
    // // command_byte.source = source;
    // Send to command queue
    if (command_byte_queue) {
        xQueueSendToBackFromISR(command_byte_queue, &ch, NULL);
    }
}

//if we recieve smt on uart and radio at the same time, this may cause UB as bytes can get mixed up
//stopgap implementation, we should just parse the packet wholesale in the future
void parse_radio_packet(uint8_t* packet, size_t packet_size){
    if (command_byte_queue) {
        for (int i = 0; i < packet_size; i++ ){
            xQueueSendToBack(command_byte_queue, &packet[i], portMAX_DELAY);
        }
    }
}

void parse_command_packet(ccsds_header_t header, uint8_t* payload_buf, uint32_t payload_size) {
    logln_info("Received command with APID: %hu", header.apid);
    switch (header.apid) {
        case CHANGE_HEARTBEAT_TELEM_RATE:
            if (payload_size < sizeof(change_heartbeat_telem_rate_t)) break;
            change_heartbeat_telem_rate_t* args = payload_buf;
            edit_steve_job_recur_time(HEARTBEAT_JOB_NAME, args->ms);
            break;
        default:
            logln_error("Received command with unknown APID: %hu", header.apid);
    }
}

void command_task(void* unused_arg) {
    // Initialize byte queue
    command_byte_queue = xQueueCreate(COMMAND_MAX_QUEUE_ITEMS, sizeof(command_byte_t));
    while (true) {
        // Keep gathering bytes until we get the sync bytes
        uint32_t sync_index = 0;
        while (true) {
            // stall thread until we get a byte
            command_byte_t command_byte = 0;
            xQueueReceive(command_byte_queue, &command_byte, portMAX_DELAY);
            
            // logln_info("Byte Received: 0x%x", command_byte);
            // logln_info("Checking Sync Byte: 0x%x", COMMAND_SYNC_BYTES[sync_index]);

            // check if current sync index byte matches
            if (command_byte != COMMAND_SYNC_BYTES[sync_index]) {
                // match unsuccessful
                sync_index = 0;
                continue;
            }
            // match successful
            // see if we've hit all sync bytes and can start parsing the packet
            if (sync_index >= (sizeof(COMMAND_SYNC_BYTES) - 2)) {
                break;
            }
            // otherwise keep checking bytes
            sync_index += 1;
        }
        // We've succesfully received all sync bytes if we've reached here
        // TODO: Add better error checks and handling here
        // Gather spacepacket header bytes
        uint8_t spacepacket_header_bytes[6];
        for (int i = 0; i < CCSDS_ENCODED_HEADER_SIZE; i++) {
            xQueueReceive(command_byte_queue, &spacepacket_header_bytes[i], portMAX_DELAY);
        }
        // Parse spacepacket header
        ccsds_header_t header = parse_ccsds_header(spacepacket_header_bytes);
        // Allocate correct size buffer
        size_t payload_size = header.packet_length + 1; // 4.1.3.5.3 transmits data size field as payload_length - 1
        uint8_t* payload_buf = pvPortMalloc(payload_size);
        if (payload_buf == 0) {
            logln_error("Failed to allocate payload buf of size 0x%x!", payload_size);
            continue;
        }
        // Read payload
        for (int i = 0; i < payload_size; i++) {
            xQueueReceive(command_byte_queue, &payload_buf[i], portMAX_DELAY);
        }
        // Parse packet payload
        parse_command_packet(header, payload_buf, payload_size);
        // Free payload buffer
        vPortFree(payload_buf);
    }
}