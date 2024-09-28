#include "usb_uart.h"

#include <stdbool.h>
#include <time.h>

int writebytes_usb(uint8_t *buffer, size_t size) {
    return write(1, buffer, size);
}

int readbytes_usb(uint8_t *buffer, size_t size) {
    do {
        int retsize = read(0, buffer, size);
        if (retsize > 0) {
            return retsize;
        }
        const struct timespec req = {
            .tv_sec = 0,
            .tv_nsec = 1000,
        };

        nanosleep(&req, NULL);
    } while (true);
}