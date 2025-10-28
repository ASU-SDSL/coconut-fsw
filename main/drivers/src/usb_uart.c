#include <unistd.h>
#include "pico/stdio_usb.h"

#include "usb_uart.h"

int writebytes_usb(uint8_t *buffer, size_t size) {
    // if the usb device is connected, send the data
    if(stdio_usb_connected()){
        return write(1, buffer, size);
    }
    // otherwise don't 
    return -1; 
}

int readbytes_usb(uint8_t *buffer, size_t size) {
    return read(0, buffer, size);
}
