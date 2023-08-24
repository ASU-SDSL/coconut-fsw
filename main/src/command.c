#include "command.h"

void receive_command_byte_from_isr(char ch) {
    // ONLY USE FROM INTERRUPTS, CREATE NEW METHOD FOR QUEUEING TASKS FROM TASKS
    // // Build command_char struct
    // command_byte_t command_byte;
    // command_byte.value = ch;
    // // command_byte.source = source;
    // Send to command queue
    if (command_byte_queue) {
        xQueueSendToBackFromISR(command_byte_queue, &ch, NULL);
    }
}

void parse_command_packet(ccsds_header_t header, uint8_t* payload_buf, uint32_t payload_size) {
    // Log apid
    char log_str[256];
    sprintf(log_str, "APID: 0x%x", header.apid);
    log_info(log_str);

    // Log payload
    sprintf(log_str, "Payload: %s", payload_buf);
    log_info(log_str);
}

void command_task(void* unused_arg) {
    // Initialize byte queue
    command_byte_queue = xQueueCreate(COMMAND_MAX_QUEUE_ITEMS, sizeof(command_byte_t));
    command_byte_t command_byte;
    while (true) {
        // Check queue
        xQueueReceive(command_byte_queue, &command_byte, portMAX_DELAY);
        // Keep gathering bytes until we get the sync bytes
        uint32_t sync_index = 0;
        while (true) {
            // stall thread until we get a byte
            uint8_t command_byte = 0;
            xQueueReceive(command_byte_queue, &command_byte, portMAX_DELAY);
            // check if current sync index byte matches
            if (command_byte != COMMAND_SYNC_BYTES[sync_index]) {
                // match unsuccessful
                sync_index = 0;
                continue;
            }
            // match successful
            // see if we've hit all sync bytes and can start parsing the packet
            if (sync_index == (sizeof(COMMAND_SYNC_BYTES) - 1)) {
                break;
            }
            // otherwise keep checking bytes
            sync_index += 1;
        }
        // We've succesfully received all sync bytes if we've reached here
        // TODO: Add better error checks and handling here
        // Gather spacepacket header bytes
        uint8_t spacepacket_header_bytes[6];
        for (int i = 0; i < CCSDS_HEADER_SIZE; i++) {
            xQueueReceive(command_byte_queue, &spacepacket_header_bytes[i], portMAX_DELAY);
        }
        // Parse spacepacket header
        ccsds_header_t header = parse_ccsds_header(spacepacket_header_bytes);
        // Allocate correct size buffer
        size_t payload_size = header.packet_length;
        uint8_t* payload_buf = pvPortMalloc(payload_size);
        if (payload_buf == 0) {
            log_error("Failed to allocate payload buf!");
            continue;
        }
        // Read payload
        for (int i = 0; i < payload_size; i++) {
            xQueueReceive(command_byte_queue, &payload_buf[i], portMAX_DELAY);
        }
        // Parse packet payload
        parse_command_packet(header, payload_buf, payload_size);
        // Free payload buffer
        pPortFree(payload_buf);
    }
}