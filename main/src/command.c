#include "command.h"

void receive_command_byte_from_isr(char ch) {
    // ONLY USE FROM INTERRUPTS, CREATE NEW METHOD FOR QUEUEING TASKS FROM TASKS
    // Build command_char struct
    command_byte_t command_byte;
    command_byte.value = ch;
    // command_byte.source = source;
    // Send to command queue
    if (command_byte_queue) {
        xQueueSendToBackFromISR(command_byte_queue, &command_byte, NULL);
    }
}

void parse_command(void* cmd, uint32_t size) {
    // TODO: Parse command packet
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
        // allocate receiving buffer
        uint32_t buf_size = STARTING_COMMAND_BUF_SIZE;
        uint32_t buf_index = 0;
        uint8_t* buf = pvPortMalloc(buf_size);
        while (true) {
            // stall thread until we get a byte
            uint8_t command_byte = 0;
            xQueueReceive(command_byte_queue, &command_byte, portMAX_DELAY);
            // check if current sync index byte matches
            if (command_byte != COMMAND_SYNC_BYTES[sync_index]) {
                // match unsuccessful
                sync_index = 0;
                // store non-sync byte
                buf[buf_index] = command_byte;
                buf_index += 1;
                // check if we need to resize buf
                if (buf_index >= buf_size) {
                    // allocate new buffer that is double the size of the old buffer
                    uint32_t new_buf_size = buf_size * 2;
                    uint8_t* new_buf = pvPortMalloc(new_buf_size);
                    /* TODO: possibly add max buffer size */
                    // copy data from old buffer
                    memcpy(new_buf, buf, buf_size);
                    // free old buffer
                    vPortFree(buf);
                    // replace old buffer with new buffer
                    buf = new_buf;
                    buf_size = new_buf_size;
                }
                continue;
            }
            // match successful
            // see if we've hit all sync bytes and can start parsing the packet
            if (sync_index == (len(COMMAND_SYNC_BYTES) - 1)) {
                break;
            }
            // otherwise keep checking bytes
            sync_index += 1;
        }
        // Parse commands
        parse_command(buf, buf_size);
    }
}