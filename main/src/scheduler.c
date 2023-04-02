#include "scheduler.h"
#include "hardware/uart.h"

// Utility Functions
TickType_t ms_to_ticks(unsigned long ms) {
    return ms / portTICK_PERIOD_MS;
}

unsigned long secs_to_ms(unsigned long secs) {
    return secs * MS_IN_SEC;
}

unsigned long mins_to_secs(unsigned long mins) {
    return mins * SECS_IN_MIN;
}

// Recurring Routine Helper Functions
// These routines will wait a specified time, run, wait a specified time, run, etc.
void schedule_recurring_routine_ms(const char* routine_name, routine_func routine_func_ptr, unsigned long ms_until_recur) {
    // Convert ms to ticks
    TickType_t recur_time = ms_to_ticks(ms_until_recur);
    // Calculate first run time 
    TickType_t execute_time = xTaskGetTickCount() + recur_time;
    // Create routine
    create_scheduler_routine(routine_name, execute_time, recur_time, routine_func_ptr);
}

void schedule_recurring_routine_secs(const char* routine_name, routine_func routine_func_ptr, unsigned long secs_until_recur) {
    schedule_recurring_routine_ms(routine_name, routine_func_ptr, secs_to_ms(secs_until_recur));
}

void schedule_recurring_routine_mins(const char* routine_name, routine_func routine_func_ptr, unsigned long mins_until_recur) {
    schedule_recurring_routine_secs(routine_name, routine_func_ptr, mins_to_secs(mins_until_recur));
}

// Delayed Routine Helper Functions
// These tasks will wait a specified time, run, then never run again
void schedule_delayed_routine_ms(const char* routine_name, routine_func routine_func_ptr, unsigned long ms_delay) {
    // Convert ms to ticks
    TickType_t tick_delay = ms_to_ticks(ms_delay);
    // Calculate first run time 
    TickType_t execute_time = xTaskGetTickCount() + tick_delay;
    // Create routine
    create_scheduler_routine(routine_name, execute_time, 0, routine_func_ptr);
}

void schedule_delayed_routine_secs(const char* routine_name, routine_func routine_func_ptr, unsigned long secs_delay) {
    schedule_delayed_routine_ms(routine_name, routine_func_ptr, secs_to_ms(secs_delay));
}

void schedule_delayed_routine_mins(const char* routine_name, routine_func routine_func_ptr, unsigned long mins_delay) {
    schedule_delayed_routine_secs(routine_name, routine_func_ptr, mins_to_secs(mins_delay));
}

// Internal Scheduler Functions
void create_scheduler_routine(const char* routine_name, TickType_t execute_time, TickType_t recur_time, routine_func routine_func_ptr) {
    // Allocate memory
    scheduler_routine* sr = pvPortMalloc(sizeof(scheduler_routine));
    // Check name and copy
    size_t routine_name_len = strlen(routine_name);
    if (routine_name_len > MAX_ROUTINE_NAME_LEN) {
        // TODO: Log error
        return;
    }
    strncpy(sr->routine_name, routine_name, routine_name_len);
    // Copy other fields
    sr->execute_time = execute_time;
    sr->recur_time = recur_time;
    sr->routine_func_ptr = routine_func_ptr;
    // Spinlock until scheduler is initialized
    while (g_scheduler_context.mutex == NULL) {
        vTaskDelay(ms_to_ticks(SCHEDULER_CHECK_DELAY_MS));
    }
    // Acquire mutex
    xSemaphoreTake(g_scheduler_context.mutex, portMAX_DELAY);
    // Add routine to list
    g_scheduler_context.routines[g_scheduler_context.routine_count++] = sr;
    // Release mutex
    xSemaphoreGive(g_scheduler_context.mutex);
}

bool run_scheduler_routine(scheduler_routine* routine) {
    // Run the routine function
    routine->routine_func_ptr();
    // Check if it needs to be rescheduled (recurring routine)
    if (routine->recur_time > 0) {
        routine->execute_time = xTaskGetTickCount() + routine->recur_time;
        return false; // returns false, the routine list does not need a cleanup
    }
    // Otherwise delete the routine
    else {
        delete_scheduler_routine(routine);
        return true; // returns true, the routine list needs a cleanup
    }
}

void delete_scheduler_routine(scheduler_routine* routine) {
    // Find routine in list and set routine ptr to null to prevent UAF's
    bool found_ptr = false;
    for (int i = 0; i < g_scheduler_context.routine_count; i++) {
        if (g_scheduler_context.routines[i] == routine) {
            found_ptr = true;
            g_scheduler_context.routines[i] = NULL;
            break;
        }
    }
    // If it couldn't find the pointer, something is wrong
    if (!found_ptr) {
        // TODO: Log error
        return;
    }
    // Free the struct memory
    vPortFree(routine);
}

void cleanup_scheduler_routines_list() {
    // Find null entries in the scheduler list
    for (int i = 0; i < g_scheduler_context.routine_count; i++) {
        if (g_scheduler_context.routines[i] == NULL) {
            // Swap the null routine with the last routine in the list
            g_scheduler_context.routines[i] = g_scheduler_context.routines[g_scheduler_context.routine_count - 1];
            g_scheduler_context.routines[g_scheduler_context.routine_count - 1] = NULL;
            // Decrement routine size counter
            g_scheduler_context.routine_count -= 1;
        }
    }
}

void initialize_scheduler_context() {
    // Set up semaphore
    g_scheduler_context.mutex = xSemaphoreCreateMutex();
    // Initialize scheduler routine memory
    memset(g_scheduler_context.routines, 0, sizeof(g_scheduler_context.routines));
    // Set routine count to zero
    g_scheduler_context.routine_count = 0;
}

// Main thread routine for scheduler task
void scheduler_task(void* unused_arg) {
    // Setup context struct
    initialize_scheduler_context();
    // Run main task loop
    while (true) {
        // Acquire mutex
        xSemaphoreTake(g_scheduler_context.mutex, portMAX_DELAY);
        // Check each routine to see if it needs to be executed
        bool needs_cleanup = false;
        for (int i = 0; i < g_scheduler_context.routine_count; i++) {
            // Get indexed routine
            scheduler_routine* indexed_routine = g_scheduler_context.routines[i];
            // Check if routine is valid
            if (indexed_routine == NULL) {
                continue;
            }
            // Check if routine is ready to run
            if (indexed_routine->execute_time >= xTaskGetTickCount()) {
                // Run the routine if so
                needs_cleanup |= run_scheduler_routine(indexed_routine);
            }
        }
        // Cleanup routine list if neccessary
        if (needs_cleanup) {
            cleanup_scheduler_routines_list();
        }
        // Release mutex
        xSemaphoreGive(g_scheduler_context.mutex);
        // Sleep for a bit before checking again
        vTaskDelay(ms_to_ticks(SCHEDULER_CHECK_DELAY_MS));
    }
}