#include "telemetry.h"

void send_telemetry(telemetry_apid_t apid, const char* payload_buffer, size_t payload_size) {
    // Build transmission buffer struct
    telemetry_queue_transmission_t telemetry;
    telemetry.apid = apid;
    telemetry.payload_buffer = pvPortMalloc(payload_size);
    memcpy(telemetry.payload_buffer, payload_buffer, payload_size);
    telemetry.payload_size = payload_size;
    // Queue telemetry packet
    if (telemetry_queue) {
        xQueueSendToBack(telemetry_queue, &telemetry, portMAX_DELAY);
    }
}

void telemetry_task(void* unused_arg) {
    // Initialize telemetry context
    g_packet_sequence_number = 0;
    // Initialize telemetry queue
    telemetry_queue = xQueueCreate(TELEMETRY_MAX_QUEUE_ITEMS, sizeof(telemetry_queue_transmission_t));
    telemetry_queue_transmission_t telemetry;
    while (true) {
        // Wait on a message in the queue
        xQueueReceive(telemetry_queue, &telemetry, UART_QUEUE_CHECK_TIME);
        // Create spacepacket header
        ccsds_header_t header;
        // Fill spacepacket header
        header.version = 0;
        header.type = 0; // telemetry type
        header.secondary_header_flag = 0;
        header.apid = telemetry.apid;
        header.sequence_flag = 3; // no segmentation
        header.packet_sequence_count = g_packet_sequence_number++;
        header.packet_length = telemetry.payload_size - 1; // 4.1.3.5.3 in spacepacket standard says packet_length - 1
        // Encode spacepacket header into bytes
        size_t header_size = TELEMETRY_SYNC_SIZE + CCSDS_ENCODED_HEADER_SIZE;
        size_t total_payload_size = header_size + telemetry.payload_size;
        char* payload_buffer = pvPortMalloc(total_payload_size);
        // Write sync bytes first
        memcpy(payload_buffer, TELEMETRY_SYNC_BYTES, TELEMETRY_SYNC_SIZE);
        // Write spacepacket header after sync bytes
        if (!encode_ccsds_header(header, payload_buffer + TELEMETRY_SYNC_SIZE)) {
            logln_error("Failed to encode SpacePacket header!");
            continue;
        }
        // Append payload to sync bytes and header
        memcpy(payload_buffer + header_size, telemetry.payload_buffer, telemetry.payload_size);
        // Send telemetry through UART
        uart_queue_message(payload_buffer, total_payload_size);

        // TODO: Send payload through radio
        // radio_queue_message(payload_buffer, total_payload_size);
        // Free buffers
        vPortFree(payload_buffer);
        vPortFree(telemetry.payload_buffer);
    }
}