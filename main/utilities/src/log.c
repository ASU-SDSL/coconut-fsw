#include <string.h>
#include <stdio.h>
#include <stdarg.h>
#include <unistd.h>
#include <string.h>

#include "FreeRTOS.h"
#include "queue.h"
#include "task.h"

#include "telemetry.h"
#include "log.h"
#include "filesystem.h"

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

    if (strncmp(packet->str, "[ERROR]", 7) == 0) {
        printf("\n IS ERROR: %s", packet->str);
        // Write error to file without "[ERROR]"
        write_error_log(packet->str);
    }

#ifdef SIMULATOR
    // write to stdout
    write(1, packet->str, strsize);
    //send_telemetry(LOG, (char*)packet, sizeof(log_telemetry_t) + strsize + 1);
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

void write_error_log(char *str) {

    // First, truncate string for log storage
    if (strlen(str) > MAX_ERROR_LOG_STR_SIZE) {
        str[MAX_ERROR_LOG_STR_SIZE] = '\0';
    }

    // See if tlm directory exists
    if (!dir_exists("/logs")) {
        make_dir("/logs");

        // Write to new log file and add this string
        printf("STRING LEN: %d\n", strlen(str));
        write_file("/logs/errors.txt", str, strlen(str) + 1, false); // +1 for end of string

        char errorlog_buf2[(MAX_ERROR_LOG_STR_SIZE + 2) * MAX_ERROR_LOG_MESSAGES]; // +1 because there is no \0 but there will be the \n
        int bytes_read2 = read_file("/logs/errors.txt", errorlog_buf2, (MAX_LOG_STR_SIZE + 1) * 5);
        if (bytes_read2 < 0) {
            logln_error("Error reading errors.txt here");
            return;
        }
        printf("Error log file:\n%s\n\n\n", errorlog_buf2);

        return;
    }

    // Else - read the current log file and append to it - remove lines if it is too long - only keep 4 lines of errors at a time
    char errorlog_buf[(MAX_ERROR_LOG_STR_SIZE + 2) * MAX_ERROR_LOG_MESSAGES]; // +1 because there is no \0 but there will be the \n
    int bytes_read = read_file("/logs/errors.txt", errorlog_buf, (MAX_LOG_STR_SIZE + 1) * 5);
    if (bytes_read < 0) {
        logln_error("Error reading errorlog.txt");
        return;
    }

    // Append str to the end of buff - logs already have newlines
    strcat(errorlog_buf, str);

    // Count the number of lines
    int line_count = 0;
    for (int i = 0; i < strlen(errorlog_buf); i++) {
        if (errorlog_buf[i] == '\n') {
            line_count++;
        }
    }

    // If there are more than MAX_ERROR_LOG_MESSAGES lines, remove the first line
    if (line_count > MAX_ERROR_LOG_MESSAGES) {
        char *first_newline = strchr(errorlog_buf, '\n'); // Get first newline
        if (first_newline != NULL) {
            memmove(errorlog_buf, first_newline + 1, strlen(first_newline));
        }
    }

    // Rewrite to file
    write_file("/logs/errors.txt", errorlog_buf, strlen(errorlog_buf), false);


    printf("ERROR logs: ");
    list_dir("/logs");

    // For testing purposes, read and print the error log file
    char errorlog_buf2[(MAX_ERROR_LOG_STR_SIZE + 2) * MAX_ERROR_LOG_MESSAGES]; // +1 because there is no \0 but there will be the \n
    int bytes_read2 = read_file("/logs/errors.txt", errorlog_buf2, (MAX_LOG_STR_SIZE + 1) * 5);
    if (bytes_read2 < 0) {
        logln_error("Error reading errors.txt here");
        return;
    }
    printf("Error log file:\n%s\n\n\n", errorlog_buf2);

}