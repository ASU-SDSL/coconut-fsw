#include "watchdog.h"
#include <stdint.h>
#include "pico/stdlib.h"
#include "hardware/gpio.h"
#include "log.h"

#define WDI_PIN 21 // from FCR schematic


void watchdog_freeze() {
    xTaskNotifyGive(xWatchdogTaskHandler);
}

void watchdog_task(void *pvParameters) {

    // Initialize the watchdog GPIO and set it high
    gpio_init(WDI_PIN);
    gpio_set_dir(WDI_PIN, GPIO_OUT);
    gpio_put(WDI_PIN, 1);

    // Simple loop to toggle gpio
    uint8_t state = 1;
    while (1) {
        logln_info("Watchdog task loop");

        // See if reset signal was received
        if (ulTaskNotifyTake(pdTRUE, 0) > 0) {
            // Freeze watchdog toggling to cause a reset
            break;
        }

        state = !state; // Flip sate to toggle on and off
        gpio_put(WDI_PIN, state);
        vTaskDelay(500);

    }

}
