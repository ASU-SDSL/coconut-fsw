#include "log.h"
#include <pico/stdio.h>

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

void _log(const char *str, ...) {

    // alloc telemetry packet
    log_telemetry_t *packet = pvPortMalloc(sizeof(log_telemetry_t) + MAX_LOG_STR_SIZE + 1);

    // copy str to packet
    va_list args;
    va_start(args, str);
    size_t strsize = vsnprintf(packet->str, MAX_LOG_STR_SIZE, str, args);
    // size_t strsize = vprintf(args, str);
    packet->size = strsize;
    va_end(args);

    // send to telemetry task
    send_telemetry(LOG, (char*)packet, sizeof(log_telemetry_t) + strsize + 1);

    vPortFree(packet);
    
}


void _write_log(const char *bytes, size_t size) {
    log_telemetry_t packet;
    memcpy(packet.str, bytes, size);
    packet.size = size;
    send_telemetry(LOG, (char*)&packet, sizeof(log_telemetry_t) + size);
}
