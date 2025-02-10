#pragma once

/* DEFINES */
#define MAX_LOG_STR_SIZE 0x1000U

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

/* INTERNAL FUNCTIONS */

void _log(const char *str, ...);

void _write_log(const char *bytes, size_t size);
