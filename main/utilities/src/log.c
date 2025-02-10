#include <string.h>
#include <stdio.h>
#include <stdarg.h>
#include <unistd.h>

#include "FreeRTOS.h"
#include "queue.h"
#include "task.h"

#include "telemetry.h"
#include "log.h"

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

// returns a global ptr into FreeRTOS task structs, do not modify or free
const char *get_current_task_name() {
    TaskStatus_t xTaskDetails;
    vTaskGetInfo(xTaskGetCurrentTaskHandle(), &xTaskDetails, pdFALSE, eInvalid );
    return xTaskDetails.pcTaskName;
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

#ifdef SIMULATOR
    // write to stdout
    write(1, packet->str, strsize);
#else
    // send to telemetry task
    send_telemetry(LOG, (char*)packet, sizeof(log_telemetry_t) + strsize + 1);
#endif

    vPortFree(packet);
}


void _write_log(const char *bytes, size_t size) {

    	log_telemetry_t packet;
    memcpy(packet.str, bytes, size);
    packet.size = size;
    send_telemetry(LOG, (char*)&packet, sizeof(log_telemetry_t) + size);
}
