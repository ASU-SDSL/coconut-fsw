#pragma once

#include <stdarg.h>

#include "FreeRTOS.h"

#include "telemetry.h"

/* DEFINES */
#define MAX_LOG_STR_SIZE 0x1000U

/* USER FUNCTIONS */

// Initialization
//  Only needs to be called once

void init_debug_log();

void print_banner();

// Use these!

#define logln_info(f_, ...) \
    _log("[INFO] " f_, ##__VA_ARGS__);

#define logln_warn(f_, ...) \
    _log("[WARN] " f_, ##__VA_ARGS__);

#define logln_error(f_, ...) \
    _log("[ERROR] " f_, ##__VA_ARGS__);

#define logln(f_, ...) \
    _log(f_ "\n", ##__VA_ARGS__);

/* INTERNAL FUNCTIONS */

void _log(char* str, ...);
