#pragma once

#include <unistd.h>
#include <stdio.h>

struct stdio_driver {
    void (*out_chars)(const char *buf, int len);
    void (*out_flush)(void);
    int (*in_chars)(char *buf, int len);
    void (*set_chars_available_callback)(void (*fn)(void*), void *param);
};

typedef struct stdio_driver stdio_driver_t;

static int stdio_usb_in_chars(char *buf, int length) {
    // return fread(buf, 0, length, stdin);
    size_t size = -1;
    do {
        size = read(0, buf, length);
        // size = fread();
    } while (size == -1);
    return size;
}

static void stdio_usb_out_chars(const char *buf, int length) {
    write(1, buf, length);
}


static stdio_driver_t stdio_usb = {
    .out_chars = stdio_usb_out_chars,
    .in_chars = stdio_usb_in_chars
};


// extern stdio_driver_t stdio_usb;
