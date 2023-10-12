#include "log.h"
#include <stdio.h>

// TODO: Add ability to use format strings with these log messages

void init_debug_log() {
    // Enable UART over USB
    stdio_init_all();
}

void print_banner() {
    _logln(
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

// void logln_info(char* str, ...) {
//     _log("[INFO] ");
//     _logln(str);
// }

// void logln_warn(char* str, ...) {
//     _log("[WARN] ");
//     _logln(str);
// }

// void logln_error(char* str, ...) {
//     _log("[ERROR] ");
//     _logln(str);
// }

// void _logln(char* str, ...) {
//     // Writes a line to the debug log
//     _log(str);
//     _log("\n");
// }

void _log(char* str, ...) {
    va_list args;
    va_start(args, str);

    // Writes to the debug log
    // Currently just printf's to the USB UART port but we can make it send telemetry in the future
    printf(str, args);

    va_end(args);
}
