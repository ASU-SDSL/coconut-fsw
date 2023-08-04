#ifndef LOG_FILE_DEFINED
#define LOG_FILE_DEFINED

/* USER FUNCTIONS */

// Initialization
// Only needs to be called once

void init_debug_log();

void print_banner();

// Use these!

void log_info(char* str);

void log_warn(char* str);

void log_error(char* str);

/* INTERNAL FUNCTIONS */

void _log(char* str);

#endif /* !LOG_FILE_DEFINED */