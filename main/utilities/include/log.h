#pragma once

/* DEFINES */
#define MAX_LOG_STR_SIZE 0x1000U
#define MAX_ERROR_LOG_STR_SIZE 0x100U // More limited as it is to be logged in our MRAM - used only for writing the logs to files
#define MAX_ERROR_LOG_MESSAGES 3 // Max number of error messages that can be logged


/* USER FUNCTIONS */

// Initialization
//  Only needs to be called once

void init_debug_log();

void print_banner();

// Use these!

// Used for general printing to console
#define log_gen(f_, ...) \
    logln(f_, ##__VA_ARGS__);

#define logln_info(f_, ...) \
    logln("[INFO] " f_, ##__VA_ARGS__);

#define logln_warn(f_, ...) \
    logln("[WARN] " f_, ##__VA_ARGS__);

#define logln_error(f_, ...) \
    logln("[ERROR] " f_, ##__VA_ARGS__);

#define logln(f_, ...) \
    _log(f_ "\n", ##__VA_ARGS__);

const char *get_current_task_name();

/*
*  Get most recent logged error
*  Log is output to out_log_str
*  Returns length of the out_log_str
*/
int get_most_recent_logged_error(char *out_log_str);

/* INTERNAL FUNCTIONS */

void _log(const char *str, ...);

void _write_log(const char *bytes, size_t size);

// ONLY for use within logln_error
void write_error_log(char *str);
