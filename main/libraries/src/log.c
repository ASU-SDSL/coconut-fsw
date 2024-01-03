#include "log.h"
#include <stdio.h>
#include <pico/stdio.h>

// TODO: Add ability to use format strings with these log messages

void init_debug_log() {
    // Enable UART over USB
    stdio_init_all();
}

void print_banner() {
    _logln(
        "=============================\n"
        "╔═╗╔═╗╔═╗╔═╗╔╗╔╦ ╦╔╦╗        \n"
        "║  ║ ║║  ║ ║║║║║ ║ ║         \n"
        "╚═╝╚═╝╚═╝╚═╝╝╚╝╚═╝ ╩         \n"
        "╔═╗╦ ╦╔╗ ╔═╗╔═╗╔═╗╔╦╗        \n"
        "║  ║ ║╠╩╗║╣ ╚═╗╠═╣ ║         \n"
        "╚═╝╚═╝╚═╝╚═╝╚═╝╩ ╩ ╩         \n"
        "Sun Devil Satellite Lab 🝰   \n"
        "=============================\n"                    
    );
}

void _log(char* str, ...) {
    va_list args;
    va_start(args, str);

    // Writes to the debug log
    // Currently just printf's to the USB UART port but we can make it send telemetry in the future
    printf(str, args);

    va_end(args);
}
