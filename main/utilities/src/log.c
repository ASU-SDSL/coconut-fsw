#include "log.h"
#include <stdio.h>
#include <pico/stdio.h>

// TODO: Add ability to use format strings with these log messages

void init_debug_log() {
    // Enable UART over USB
    stdio_init_all();
}

void print_banner() {
    logln(
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

    // alloc telemetry packet
    log_telemetry_t *packet = pvPortMalloc(sizeof(log_telemetry_t) + MAX_LOG_STR_SIZE + 1);

    // copy str to packet
    size_t strsize = vsnprintf(packet->str, MAX_LOG_STR_SIZE, str, args);
    packet->size = strsize;
    
    va_end(args);

    // send to telemetry task
    send_telemetry(LOG, (char*)packet, sizeof(log_telemetry_t) + strsize + 1);

}
