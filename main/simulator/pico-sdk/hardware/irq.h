#pragma once

#include <stdbool.h>

typedef void (*irq_handler_t)(void);

void static irq_set_exclusive_handler(uint32_t num, irq_handler_t handler) {
    return;
}

void static irq_set_enabled(uint32_t num, bool enabled) {
    return;
}

