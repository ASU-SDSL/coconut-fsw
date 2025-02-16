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

    logln_info("test: %s", str);

    // First, truncate string for log storage
    if (strlen(str) > MAX_ERROR_LOG_STR_SIZE) {
        str[MAX_ERROR_LOG_STR_SIZE] = '\0';
    }

    // Remove "[ERROR] " (8 chars)
    str += 8;

    logln_info("here4");
    // See if tlm directory exists
    if (!dir_exists("/logs")) {
        logln_info("here5");
        make_dir("/logs");
        logln_info("here6");

        // Write to new log file and add this string
        write_file("/logs/errors.txt", str, strlen(str) + 1, false); // +1 for end of string

        return;
    }

    logln_info("test12");

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
    for (int i = 0; i < (int) strlen(errorlog_buf); i++) {
        if (errorlog_buf[i] == '\n') {
            line_count++;
        }
    }

    // If there are more than MAX_ERROR_LOG_MESSAGES lines, remove the first line
    char *errlog_buf_ptr;
    if (line_count > MAX_ERROR_LOG_MESSAGES) {
        char *first_newline = strchr(errorlog_buf, '\n'); // Get first newline
        if (first_newline != NULL) {
            errlog_buf_ptr = first_newline + 1;
        } else {
            errlog_buf_ptr = errorlog_buf;
        }
    } else {
        errlog_buf_ptr = errorlog_buf;
    }

    // Rewrite to file
    write_file("/logs/errors.txt", errlog_buf_ptr, strlen(errlog_buf_ptr) + 1, false); // +1 for end string char

    printf("ERROR logs: ");
    list_dir("/logs");

    // For testing purposes, read and print the error log file
    /*char errorlog_buf2[(MAX_ERROR_LOG_STR_SIZE + 2) * MAX_ERROR_LOG_MESSAGES]; // +1 because there is no \0 but there will be the \n
    int bytes_read2 = read_file("/logs/errors.txt", errorlog_buf2, (MAX_LOG_STR_SIZE + 1) * 5);
    if (bytes_read2 < 0) {
        logln_error("Error reading errors.txt here");
        return;
    }
    printf("Error log file:\n%s\n\n\n", errorlog_buf2);

    char stri[MAX_ERROR_LOG_STR_SIZE];
    get_most_recent_logged_error(&stri);
    printf("MOST RECENT ERROR: %s\n", stri);*/

}

// Returns length of the out_log_str
int get_most_recent_logged_error(char *out_log_str) {

    // Read the current log file and append to it - remove lines if it is too long - only keep 4 lines of errors at a time
    char errorlog_buf[(MAX_ERROR_LOG_STR_SIZE + 2) * MAX_ERROR_LOG_MESSAGES]; // +1 because there is no \0 but there will be the \n
    int bytes_read = read_file("/logs/errors.txt", errorlog_buf, (MAX_LOG_STR_SIZE + 1) * 5);
    if (bytes_read < 0) {
        logln_error("Error reading errorlog.txt");
        return -1;
    }

    // Get the last line - at the end of this loop, line_ptr will have the char after the last newline
    // Last line will have a newline, so get the newline before last
    char *line_ptr;
    char *most_recent_newline;
    for (int i = 0; i < (int) strlen(errorlog_buf); i++) {
        if (errorlog_buf[i] == '\n') {
            line_ptr = most_recent_newline;
            most_recent_newline = (char*)&errorlog_buf[i + 1];
        }
    }

    if (most_recent_newline == NULL) {
        logln_error("No errors in errorlog.txt");
        return -1;
    } else if (line_ptr == NULL) { // The first log is at the beginning of the file
        line_ptr = errorlog_buf;
    }

    line_ptr[strlen(line_ptr) - 1] = '\0'; // Remove the newline

    // Copy the error log to the log_str
    strncpy(out_log_str, line_ptr, strlen(line_ptr) + 1); // + 1 for endstring
    return strlen(line_ptr);

}
