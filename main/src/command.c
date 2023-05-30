#include "command.h"

void receive_byte_from_transmission(char byte) {
    
}

void receive_command_from_isr(transmission_buffer command) {
    // NOTICE: ONLY USE FROM INTERRUPTS, CREATE NEW METHOD FOR QUEUEING TASKS FROM TASKS
    // Allocate buffer
    transmission_buffer* allocated_buffer = malloc(sizeof(transmission_buffer));
    // Queue buffer to command queue
    xQueueSendToBack(command_queue, &allocated_buffer, portMAX_DELAY);
}

void command_task(void* unused_arg) {
    // Initialize command queue
    command_queue = xQueueCreate(COMMAND_MAX_QUEUE_ITEMS, sizeof(transmission_buffer));
    transmission_buffer command;
    while (true) {
        // Wait on a message in the queue
        xQueueReceive(command_queue, &command, UART_QUEUE_CHECK_TIME);
        // TODO: Process spacepacket
    }
}