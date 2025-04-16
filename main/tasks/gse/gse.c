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
#include "miscellaneous_jobs.h"

void gse_queue_message(char* buffer, size_t size) {
    // write to picosdk usb uart interface
    writebytes_usb(buffer, size);
}

void gse_task(void *pvParameters) {
    // Initialize USB UART
    stdio_init_all();

    for(int i = 0; i < 100; i++){
        buzzer_beep_job(); 
        vTaskDelay(pdMS_TO_TICKS(500)); 
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
