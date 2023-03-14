#include "gse.h"
#include <stdio.h>
#include "pico/stdlib.h"

/**
 * @brief Monitor GSE UART and send messages if needed
 */
void gse_task(void* unused_arg) {

    while (true) {

        // do stuff

        const uint LED_PIN = PICO_DEFAULT_LED_PIN;
        gpio_init(LED_PIN);
        gpio_set_dir(LED_PIN, GPIO_OUT);
        while (true) {
            gpio_put(LED_PIN, 1);
            vTaskDelay(1000);
            gpio_put(LED_PIN, 0);
            vTaskDelay(1000);
        }
    }

}
