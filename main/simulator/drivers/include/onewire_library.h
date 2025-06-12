#pragma once

#include "pico/stdlib.h"
#include <stdint.h>
#include <stdbool.h>
#include "hardware/pio.h"

const uint32_t onewire_program = 0; // Placeholder for the onewire PIO program defined in the library

// Blank struct for basic stub
typedef struct {
} OW;

bool ow_init(OW *ow, PIO pio, uint offset, uint gpio) {
    return true;
}

void ow_send(OW *ow, uint data) {
    return;
}

uint8_t ow_read(OW *ow) {
    return 1; // Sample return value
}

bool ow_reset(OW *ow) {
    return true;
}

int ow_romsearch(OW *ow, uint64_t *romcodes, int maxdevs, uint command) {
    return 0;
}
