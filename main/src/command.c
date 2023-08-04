#include "command.h"

void receive_command_byte_from_isr(char ch, command_source_t source) {
    // ONLY USE FROM INTERRUPTS, CREATE NEW METHOD FOR QUEUEING TASKS FROM TASKS
    // Build command_char struct
    command_byte_t command_byte;
    command_byte.value = ch;
    command_byte.source = source;
    // Send to command queue
    if (command_byte_queue) {
        xQueueSendToBackFromISR(command_byte_queue, &command_byte, NULL);
    }
}

void command_task(void* unused_arg) {
    // Initialize byte queue
    command_byte_queue = xQueueCreate(COMMAND_MAX_QUEUE_ITEMS, sizeof(command_byte_t));
    command_byte_t command_byte;
    while (true) {
        // Check queue
        xQueueReceive(command_byte_queue, &command_byte, portMAX_DELAY);
        // TODO: Join bytes into packet and process
        //       Will need a seperate packet builder/buffer for each source so bytes don't get mixed together
    }
}