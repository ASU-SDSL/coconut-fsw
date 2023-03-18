#include "scheduler.h"

// Recurring Routines
// These routines will wait a specified time, run, wait a specified time, run, etc.

void recurring_routine_task(struct recurring_routine_params* rrp) {
    while (true) {
        // Delay task for recur time
        vTaskDelay(rrp->ticks_until_recur);
        // Run routine
        rrp->recurring_routine_func_ptr();
    }
}

void schedule_recurring_routine_ms(const char* routine_name, void* routine_func_ptr, unsigned int ms_until_recur) {
    // Convert ms to ticks
    TickType_t delay = ms_until_recur / portTICK_PERIOD_MS;
    // Allocate recurring tasks parameter struct
    struct recurring_routine_params* rrp = pvPortMalloc(sizeof(struct recurring_routine_params));
    rrp->recurring_routine_func_ptr = routine_func_ptr;
    rrp->ticks_until_recur = delay;
    // Schedule reccuring task
    xTaskCreate((void (*)(void *))recurring_task_routine, routine_name, 128, NULL, 1, (void *)rrp); 
}

void schedule_recurring_routine_secs(const char* routine_name, void* routine_func_ptr, unsigned int secs_until_recur) {
    schedule_recurring_routine_ms(routine_name, routine_func_ptr, secs_until_recur * 1000);
}

void schedule_recurring_routine_mins(const char* routine_name, void* routine_func_ptr, unsigned int mins_until_recur) {
    schedule_recurring_routine_mins(routine_name, routine_func_ptr, mins_until_recur * 60);
}


// Delayed Routines
// These tasks will wait a specified time, run, then never run again
void delayed_routine_task(struct delayed_routine_params* drp) {
    // Delay task for recur time
    vTaskDelay(drp->ticks_delay);
    // Run routine
    drp->delayed_routine_func_ptr();
}

void schedule_delayed_routine_ms(const char* routine_name, void* routine_func_ptr, unsigned int ms_delay) {
    // Convert ms to ticks
    TickType_t delay = ms_delay / portTICK_PERIOD_MS;
    // Allocate delayed tasks parameter struct
    struct delayed_routine_params* drp = pvPortMalloc(sizeof(struct delayed_routine_params));
    drp->delayed_routine_func_ptr = routine_func_ptr;
    drp->ticks_delay = delay;
    // Schedule delayed task
    xTaskCreate((void (*)(void *))delayed_task_routine, routine_name, 128, NULL, 1, (void *)drp); 
}

void schedule_delayed_routine_secs(const char* routine_name, void* routine_func_ptr, unsigned int secs_delay) {
    schedule_delayed_routine_ms(routine_name, routine_func_ptr, secs_delay * 1000);
}

void schedule_delayed_routine_mins(const char* routine_name, void* routine_func_ptr, unsigned int mins_delay) {
    schedule_delayed_routine_mins(routine_name, routine_func_ptr, mins_delay * 60);
}

// // Global mutex for thread-safety
// SemaphoreHandle_t scheduler_mutex;

// // Global storage for scheduler routines
// std::vector<scheduler_routine> scheduler_queue;

// /**
//  * @brief Main thread routine for scheduler task
//  */
// void scheduler_task(void* unused_arg) {
//     // Setup mutex
//     scheduler_mutex = xSemaphoreCreateMutex();
//     while (true) {
//         // Acquire mutex
//         xSemaphoreTake(scheduler_mutex, portMAX_DELAY);
//         // Check each routine to see if it needs to be executed
//         for (scheduler_routine routine : scheduler_queue) {
            
//         }
//         // Release mutex
//         xSemaphoreGive(scheduler_mutex);
//         // Sleep for a bit before checking again
//         vTaskDelay(SCHEDULER_CHECK_DELAY);
//     }
// }