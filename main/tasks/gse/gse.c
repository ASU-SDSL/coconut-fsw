#include "gse.h"

void gse_queue_message(char* buffer, size_t size) {
    // write to picosdk usb uart interface
    writebytes_usb(buffer, size);
}

void gse_task(void *pvParameters) {
    // Initialize USB UART
    stdio_init_all();

    // Initialize read LED
    gpio_init(LED_PIN);
    gpio_set_dir(LED_PIN, GPIO_OUT);
    
    // Start listening for USB UART bytes
    telemetry_queue_transmission_t rec;
    while (true) {
        // Enable write LED
        gpio_put(LED_PIN, 1);
        // Wait on bytes from stdin
        char c;
        int len = readbytes_usb(&c, 1);  
        // Disable write LED
        gpio_put(LED_PIN, 0); 
        // Send byte to command task
        receive_command_byte(c);
    }
}
