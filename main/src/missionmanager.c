#include "missionmanager.h"
#include <stdio.h>
#include "pico/stdlib.h"

/**
 * @brief Schedule and execute tasks
 */
void missionmanager_task(void* unused_arg) {

    while (true) {

        // do stuff (feel free to remove the below, I was just using this for testing)
        vTaskDelay(1000);
        printf("Mission Manager task is running\n");
    }
}