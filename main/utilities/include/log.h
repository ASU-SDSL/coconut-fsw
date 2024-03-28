#pragma once

#include <stdarg.h>

/* USER FUNCTIONS */

// Initialization
//  Only needs to be called once

void init_debug_log();

void print_banner();

// Use these!

#define logln_info(f_, ...) \
    _log("[INFO] "); _logln((f_), ##__VA_ARGS__);

#define logln_warn(f_, ...) \
    _log("[WARN] "); _logln((f_), ##__VA_ARGS__);

#define logln_error(f_, ...) \
    _log("[ERROR] "); _logln((f_), ##__VA_ARGS__);

#define _logln(f_, ...) \
    _log((f_), ##__VA_ARGS__); _log("\n");

/* INTERNAL FUNCTIONS */

void _log(char* str, ...);
