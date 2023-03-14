#include "radio.h"
#include <stdio.h>
#include "pico/stdlib.h"

/**
 * @brief Monitor radio, write to SD card, and send stuff when needed
 */
void radio_task(void* unused_arg) {

    while (true) {

        // do stuff (feel free to remove the below, I was just using this for testing)

        vTaskDelay(100);
        printf("Radio task is running");
    }
}