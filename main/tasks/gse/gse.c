#include "gse.h"

void gse_queue_message(char* buffer, size_t size) {
    // write to picosdk usb uart interface
    writebytes_usb(buffer, size);
}

void gse_task(void *pvParameters) {
    // Initialize USB UART
    stdio_init_all();

    // Start listening for USB UART bytes
    while (true) {
        // Wait on bytes from stdin
        char c;
        int len = readbytes_usb(&c, 1);  
        // Send byte to command task
        receive_command_byte(c);
    }
}
