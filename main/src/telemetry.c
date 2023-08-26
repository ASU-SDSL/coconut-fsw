#include "telemetry.h"

void send_telemetry(const char* buffer, size_t size) {
    // Build transmission buffer struct
    transmission_buffer_t telemetry;
    telemetry.buffer = pvPortMalloc(size);
    memcpy(telemetry.buffer, buffer, size);
    telemetry.size = size;
    // Queue telemetry packet
    if (telemetry_queue) {
        xQueueSendToBack(telemetry_queue, &telemetry, portMAX_DELAY);
    }
}

void telemetry_task(void* unused_arg) {
    // Initialize telemetry queue
    telemetry_queue = xQueueCreate(TELEMETRY_MAX_QUEUE_ITEMS, sizeof(transmission_buffer_t));
    transmission_buffer_t telemetry;
    while (true) {
        // Wait on a message in the queue
        xQueueReceive(telemetry_queue, &telemetry, UART_QUEUE_CHECK_TIME);
        // TODO: Wrap contents in spacepacket
        // Send telemetry through UART
        uart_queue_message(telemetry.buffer, telemetry.size);
        // TODO: Send telemetry through radio
        // Free transmission buffer
        vPortFree(telemetry.buffer);
    }
}