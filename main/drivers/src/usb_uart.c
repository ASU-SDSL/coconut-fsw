#include <unistd.h>

#include "usb_uart.h"

int writebytes_usb(uint8_t *buffer, size_t size) {
    return write(1, buffer, size);
}

int readbytes_usb(uint8_t *buffer, size_t size) {
    return read(0, buffer, size);
}
