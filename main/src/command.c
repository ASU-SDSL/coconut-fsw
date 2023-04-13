#include "command.h"



void command_task(void* unused_arg) {
    // Initialize command queue
    command_queue = xQueueCreate(COMMAND_MAX_QUEUE_ITEMS, sizeof(command_message_type));
    command_message_type command = NULL;
    while (true) {
        // Wait on a message in the queue
        xQueueReceive(command_queue, &command, UART_QUEUE_CHECK_TIME);
        // TODO: Process command and send response
    }
}