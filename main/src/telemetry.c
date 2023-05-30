#include "telemetry.h"

void send_telemtry(transmission_buffer command) {

}

void telemetry_task(void* unused_arg) {
    // Initialize telemetry queue
    telemetry_queue = xQueueCreate(TELEMETRY_MAX_QUEUE_ITEMS, sizeof(transmission_buffer));
    transmission_buffer telemetry;
    while (true) {
        // Wait on a message in the queue
        xQueueReceive(telemetry_queue, &telemetry, UART_QUEUE_CHECK_TIME);
        // TODO: Send telemetry through UART and radio
    }
}