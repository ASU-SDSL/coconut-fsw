#include <stdint.h>
#include "pico/stdlib.h"
#include "watchdog_max706resa.h"
#include "hardware/gpio.h"

void initialize_watchdog() {
    gpio_init(WDI_PIN);
    gpio_set_dir(WDI_PIN, GPIO_OUT);
}

void set_watchdog_gpio(int state) {
    gpio_put(WDI_PIN, state);
}
