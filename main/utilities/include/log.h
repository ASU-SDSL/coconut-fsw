#pragma once

/* DEFINES */
#define MAX_LOG_STR_SIZE 0x1000U // For info and warn
// For a single error, more limited as it logged in MRAM - used only for writing the logs to files
// To find max error messages allowed to be logged, divide ERROR_LOGS_FS_ALLOCATION by MAX_ERROR_LOG_STR_SIZE (see filesystem.h)
#define MAX_ERROR_LOG_STR_SIZE 0x100U // This should be a dividend of ERROR_LOGS_FS_ALLOCATION (see filesystem.h)


//#define MAX_ERROR_LOG_MESSAGES 3 // Max number of error messages that can be logged

static const char ERROR_LOG_FILE_PATH[] = "/logs/log.txt";

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
    _log_error_fs(f_, ##__VA_ARGS__); \
    _log_error("[ERROR] " f_, ##__VA_ARGS__);

#define logln(f_, ...) \
    _log(f_ "\n", ##__VA_ARGS__);

const char *get_current_task_name();

/*
*  Get most recent logged error
*  Log is output to out_log_str
*  Returns length of the out_log_str
*/
int get_most_recent_logged_error(char *out_log_str, int out_log_str_size);

/* INTERNAL FUNCTIONS */

void _log(const char *str, ...);

void _write_log(const char *bytes, size_t size);

// ONLY for use within logln_error
// There will be a MAX of  ERROR_LOGS_FS_ALLOCATION / MAX_ERROR_LOG_STR_SIZE lines at a time logged to the error file
void write_error_log(char *str);
