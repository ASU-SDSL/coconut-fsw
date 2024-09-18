#include <stdio.h>
#include "pico/stdlib.h"
#include "ff.h"

#include "gse.h"
#include "command.h"
#include "rtc.h"

#include "eps.h"
#include "mag.h"
#include <string.h>

void gse_queue_message(char* buffer, size_t size) {
    // write to picosdk usb uart interface
    stdio_usb.out_chars(buffer, size);
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
        char c = 0;
        stdio_usb.in_chars(&c, 1);
        // Disable write LED
        gpio_put(LED_PIN, 0); 
        // Send byte to command task
        receive_command_byte(c);
    }
}
