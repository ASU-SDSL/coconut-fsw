#include "watchdog.h"
#include <stdint.h>
#include "pico/stdlib.h"
#include "hardware/gpio.h"
#include "log.h"
#include <stdlib.h>

#define WDI_PIN 21 // from FCR schematic

#define HEARTBEAT_CHECK_PERIOD_MS (10 * 1000) // 10 s
#define WATCHDOG_MUTEX_DELAY_MS 1000

/// @brief Array of task heartbeats to watch 
static task_heartbeat_t** watchlist = NULL; 
static int watchlist_len = 0; 

task_heartbeat_t* build_task_heartbeat(const char* name){
    task_heartbeat_t* thb = (task_heartbeat_t*) pvPortMalloc(sizeof(task_heartbeat_t)); 
    thb->name = name; 
    thb->tick = 0; 
    thb->mutex = xSemaphoreCreateMutex();

    // make a new larger watchlist
    task_heartbeat_t** new_watchlist = pvPortMalloc(sizeof(task_heartbeat_t*) * (watchlist_len + 1)); 

    // copy old watch list 
    for(int i = 0; i < watchlist_len; i++){
        new_watchlist[i] = watchlist[i]; 
    }
    
    // add new heartbeat
    new_watchlist[watchlist_len] = thb; 
    watchlist_len++; 

    // replace old watchlist 
    if(xSemaphoreTake(watchlist_mutex, WATCHDOG_MUTEX_DELAY_MS)) {
        vPortFree(watchlist); 
        watchlist = new_watchlist;

        xSemaphoreGive(watchlist_mutex); 
    }

    return thb; 
}


void task_heartbeat_tick(task_heartbeat_t* watchdog) {
    if(xSemaphoreTake(watchlist_mutex, WATCHDOG_MUTEX_DELAY_MS) && xSemaphoreTake(watchdog->mutex, WATCHDOG_MUTEX_DELAY_MS)) {
        watchdog->tick = true; 

        xSemaphoreGive(watchdog->mutex); 
        xSemaphoreGive(watchlist_mutex); 
    }
}

static bool task_heartbeat_read(task_heartbeat_t* watchdog){
    bool res = false; 
    if(xSemaphoreTake(watchlist_mutex, WATCHDOG_MUTEX_DELAY_MS) && xSemaphoreTake(watchdog->mutex, WATCHDOG_MUTEX_DELAY_MS)) {
        res = watchdog->tick;

        xSemaphoreGive(watchdog->mutex); 
        xSemaphoreGive(watchlist_mutex); 
    }

    return res; 
}

static void task_heartbeat_untick(task_heartbeat_t* watchdog){
    if(xSemaphoreTake(watchlist_mutex, WATCHDOG_MUTEX_DELAY_MS) && xSemaphoreTake(watchdog->mutex, WATCHDOG_MUTEX_DELAY_MS)) {
        watchdog->tick = false;

        xSemaphoreGive(watchdog->mutex); 
        xSemaphoreGive(watchlist_mutex); 
    }
}

void watchdog_freeze() {
    // Notify main loop in watchdog_task() to freeze
    xTaskNotifyGive(xWatchdogTaskHandler);
}

void watchdog_task(void *pvParameters) {

    // Initialize the watchdog GPIO and set it high
    gpio_init(WDI_PIN);
    gpio_set_dir(WDI_PIN, GPIO_OUT);
    gpio_put(WDI_PIN, 1);

    // Simple loop to toggle gpio
    uint8_t state = 1;
    uint32_t last = to_ms_since_boot(get_absolute_time());
    while (1) {

        // every defined bit 
        if(abs(to_ms_since_boot(get_absolute_time()) - last) > HEARTBEAT_CHECK_PERIOD_MS){
            // check heartbeats
            for(int i = 0; i < watchlist_len; i++){
                // if it is still false, the task has flat lined 
                if(task_heartbeat_read(watchlist[i]) == false){ 
                    // trigger restart 
                    watchdog_freeze(); 
                    break; 
                } else {
                    // reset 
                    task_heartbeat_untick(watchlist[i]); 
                }
            }
        }

        // See if reset signal was received
        if (ulTaskNotifyTake(pdTRUE, 0) > 0) {
            // Freeze watchdog toggling to cause a reset
            logln_error("Watchdog triggered, rebooting...");
            while(1) {}
        }

        state = !state; // Flip state to toggle on and off
        gpio_put(WDI_PIN, state);
        vTaskDelay(500);

    }

}
