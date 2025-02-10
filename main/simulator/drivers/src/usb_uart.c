#include <stdio.h>
#include <unistd.h>
#include <stdbool.h>

#include "FreeRTOS.h"
#include "task.h"

#include "usb_uart.h"

int writebytes_usb(uint8_t *buffer, size_t size) {
    return write(1, buffer, size);
}

int readbytes_usb(uint8_t *buffer, size_t size) {
    do {
        int retsize = read(0, buffer, size);
        if (retsize > 0) {
            return retsize;
        }
        vTaskDelay(1 / portTICK_PERIOD_MS);
    } while (true);
}
