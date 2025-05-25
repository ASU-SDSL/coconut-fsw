#include <stdint.h>
#include <stdbool.h>
#include "pico/stdlib.h"

typedef uint32_t PIO;
#define pio0 0 // Not to be used, just a placeholder

// Change program type to uint32_t instead of pico_program_t specific type
static bool pio_can_add_program(PIO pio, const uint32_t *program) {
    return true;
}

static uint pio_add_program(PIO pio, const uint32_t *program) {
    return 0;
}
