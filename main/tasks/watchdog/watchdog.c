#include "watchdog.h"
#include <stdint.h>
#include "pico/stdlib.h"
#include "hardware/gpio.h"
#include "log.h"
#include <stdlib.h>

#define WDI_PIN 21 // from FCR schematic

#define WATCHDOG_MAX_QUEUE_ITEMS 16
#define WATCHDOG_INTERTASK_CHECK_PERIOD_MS 1000 * 60 // every minute 
#define WATCHDOG_CONNECTED_TASKS 0
#define WATCHDOG_CHECK_DELAY_MS 500

static QueueHandle_t watchdog_queue;
#if WATCHDOG_CONNECTED_TASKS > 0 
static bool heartbeats[WATCHDOG_CONNECTED_TASKS]; 
#endif 

static uint32_t last_check = 0; 


void watchdog_freeze() {
    // Notify main loop in watchdog_task() to freeze
    xTaskNotifyGive(xWatchdogTaskHandler);
}

void watchdog_task(void *pvParameters) {

    // set up watchdog queue
    watchdog_queue = xQueueCreate(WATCHDOG_MAX_QUEUE_ITEMS, sizeof(uint8_t));

    // update last check 
    last_check = to_ms_since_boot(get_absolute_time()); 

    // Initialize the watchdog GPIO and set it high
    gpio_init(WDI_PIN);
    gpio_set_dir(WDI_PIN, GPIO_OUT);
    gpio_put(WDI_PIN, 1);

    // Simple loop to toggle gpio
    uint8_t state = 1;
    while (1) {

        // See if reset signal was received
        if (ulTaskNotifyTake(pdTRUE, 0) > 0) {
            // Freeze watchdog toggling to cause a reset
            while(1) {}
        }

        #if WATCHDOG_CONNECTED_TASKS > 0
        // receive intertask heartbeats from queue
        uint8_t heartbeat_index; 
        while(xQueueReceive(watchdog_queue, &heartbeat_index, 0) == pdPASS){
            heartbeats[heartbeat_index] = true; 
        }

        if(abs((long long)(to_ms_since_boot(get_absolute_time()) - last_check)) > WATCHDOG_INTERTASK_CHECK_PERIOD_MS){
            // check heartbeats 
            for(size_t i = 0; i < WATCHDOG_CONNECTED_TASKS; i++){
                // if a heartbeat hasn't toggled since last check 
                if(heartbeats[i] == false){
                    watchdog_freeze(); 
                    break; 
                }
                // reset heartbeat
                heartbeats[i] = false; 
            }
        }
        #endif 

        state = !state; // Flip state to toggle on and off
        gpio_put(WDI_PIN, state);
        vTaskDelay(500);

    }

}

void watchdog_kick(uint8_t id){
    while(!watchdog_queue)
    {
        vTaskDelay(WATCHDOG_CHECK_DELAY_MS / portTICK_PERIOD_MS); 
    }
    xQueueSendToBack(watchdog_queue, &id, portMAX_DELAY); 
}