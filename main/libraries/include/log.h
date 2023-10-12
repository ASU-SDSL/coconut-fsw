#ifndef LOG_FILE_DEFINED
#define LOG_FILE_DEFINED

#include <stdarg.h>

/* USER FUNCTIONS */

// Initialization
//  Only needs to be called once

void init_debug_log();

void print_banner();

// Use these!

#define logln_info(...) \
    _log("[INFO] "); _logln(__VA_ARGS__);

#define logln_warn(...) \
    _log("[WARN] "); _logln(__VA_ARGS__);

#define logln_error(...) \
    _log("[ERROR] "); _logln(__VA_ARGS__);

#define _logln(...) \
    _log(__VA_ARGS__); _log("\n");

// void log_info(char* str);

// void log_warn(char* str);

// void log_error(char* str);

/* INTERNAL FUNCTIONS */

void _log(char* str, ...);

#endif /* !LOG_FILE_DEFINED */