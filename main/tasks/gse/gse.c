#include <string.h>

#include "FreeRTOS.h"
#include "queue.h"
#include "pico/stdio.h"

#include "usb_uart.h"
#include "command.h"
#include "rtc_ds3231.h"
#include "ina219.h"
#include "log.h"
#include "gse.h"
#include "ds18b_onewire.h"

void gse_queue_message(char* buffer, size_t size) {
    // write to picosdk usb uart interface
    writebytes_usb(buffer, size);
}

void gse_task(void *pvParameters) {
    // Initialize USB UART
    stdio_init_all();

    while(1){
        printf("looooooop\n"); 

        float data = 0; 
        uint8_t res = ds18b_read_temp(&data);
        // if on reset read again 
        if(res == 1){
            printf("seen all devices\n"); 
            res = ds18b_read_temp(&data); 
        }

        printf("res = %d\n", res); 
        printf("data = %f\n", data); 

        // debug_sample_loop(); 
    }

    // Start listening for USB UART bytes
    while (true) {

        // Wait on bytes from stdin
        char c;
        int len = readbytes_usb(&c, 1);  
        // Send byte to command task
        receive_command_byte(c);
    }
}
