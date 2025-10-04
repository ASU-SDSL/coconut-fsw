#include <string.h>
#include <stdio.h>
#include <stdarg.h>
#include <unistd.h>
#include <string.h>

#include <FreeRTOS.h>
#include "queue.h"
#include "task.h"

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

void _log(bool is_error, const char *str, ...) {
    // alloc telemetry packet
    log_telemetry_t *packet = pvPortMalloc(sizeof(log_telemetry_t) + MAX_LOG_STR_SIZE + 1);

    // copy str to packet
    va_list args;
    va_start(args, str);
    size_t strsize = vsnprintf(packet->str, MAX_LOG_STR_SIZE, str, args);
    // size_t strsize = vprintf(args, str);
    packet->size = strsize;
    va_end(args);

    if (is_error) {
        write_error_log(packet->str); // Write to the log
    }

//#ifdef SIMULATOR
    // write to stdout
    //write(1, packet->str, strsize);
    //send_telemetry(LOG, (char*)packet, sizeof(log_telemetry_t) + strsize + 1);
//#else
    // send to telemetry task
    send_telemetry(LOG, (char*)packet, sizeof(log_telemetry_t) + strsize + 1);
//#endif

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
    if (strlen(str) >= MAX_ERROR_LOG_STR_SIZE) { // >= for the \n
        str[MAX_ERROR_LOG_STR_SIZE - 1] = '\n';
        str[MAX_ERROR_LOG_STR_SIZE] = '\0';
    }

    // Remove "[ERROR] " (8 chars)
    str += 8;

    // See if tlm directory exists
    if (!dir_exists("/logs")) {
        make_dir("/logs");

        // Write to new log file and add this string
        write_file(ERROR_LOG_FILE_PATH, str, strlen(str) + 1, false); // +1 for end of string

        return;
    }

    // Else - read the current log file and append to it - remove lines if it is too long
    // only keep ERROR_LOGS_FS_ALLOCATION / MAX_ERROR_LOG_STR_SIZE lines of errors at a time
    char errorlog_buf[MAX_ERROR_LOG_STR_SIZE]; // +1 because there is no \0 but there will be the \n
    int bytes_read = read_file(ERROR_LOG_FILE_PATH, errorlog_buf, MAX_ERROR_LOG_STR_SIZE);
    if (bytes_read < 0) {
        logln_info("Error reading error.txt");
        return;
    }

    // Append str to the end of buff - logs already have newlines and str will have max size of MAX_ERROR_LOG_STR_SIZE
    strcat(errorlog_buf, str);

    // Count the number of lines
    int line_count = 0;
    for (int i = 0; i < (int) strlen(errorlog_buf); i++) {
        if (errorlog_buf[i] == '\n') {
            line_count++;
        }
    }

    // If there are more than the max allowed error lines, remove the first line
    char *errlog_buf_ptr;
    if (line_count > ERROR_LOGS_FS_ALLOCATION / MAX_ERROR_LOG_STR_SIZE) {
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
    write_file(ERROR_LOG_FILE_PATH, errlog_buf_ptr, strlen(errlog_buf_ptr) + 1, false); // +1 for end string char

}

// Returns length of the out_log_str
int get_most_recent_logged_error(char *out_log_str, int out_log_str_size) {

    // Read the current log file and append to it - remove lines if it is too long - only keep 4 lines of errors at a time
    // read up to the max amount of allocated space for logs
    char *errorlog_buf = pvPortMalloc(ERROR_LOGS_FS_ALLOCATION);
    int bytes_read = read_file(ERROR_LOG_FILE_PATH, errorlog_buf, ERROR_LOGS_FS_ALLOCATION);
    errorlog_buf[ERROR_LOGS_FS_ALLOCATION] = '\0'; // Ensure null terminated
    if (bytes_read < 0) {
        vPortFree(errorlog_buf);
        logln_info("Error reading errors.txt");
        return -1;
    }

    // Get the last line - at the end of this loop, line_ptr will have the char after the last newline
    // Last line will have a newline, so get the newline before last
    char *line_ptr = NULL;
    char *most_recent_newline = NULL;
    for (int i = 0; i < (int) strlen(errorlog_buf); i++) {
        if (errorlog_buf[i] == '\n') {
            line_ptr = most_recent_newline;
            most_recent_newline = (char*)&errorlog_buf[i + 1];
        }
    }

    if (most_recent_newline == NULL) {
        vPortFree(errorlog_buf);
        logln_error("No errors in errors.txt");
        return -1;
    } else if (line_ptr == NULL) { // The first log is at the beginning of the file
        line_ptr = errorlog_buf;
    }

    line_ptr[strlen(line_ptr) - 1] = '\0'; // Remove the newline

    // Copy the error log to the log_str
    strncpy(out_log_str, line_ptr, out_log_str_size);
    vPortFree(errorlog_buf);
    return strlen(line_ptr);

}
