#ifndef SCHEDULER_FILE_DEFINED
#define SCHEDULER_FILE_DEFINED

#include <stdio.h>
#include <string.h>

#include <FreeRTOS.h>
#include <task.h>
#include <semphr.h>
#include "pico/stdlib.h"

// Constants
#define MAX_ROUTINES 256
#define MAX_ROUTINE_NAME_LEN 100
#define SCHEDULER_CHECK_DELAY_MS 5000
#define MS_IN_SEC 1000
#define SECS_IN_MIN 60

// Structs and Types
typedef void (*routine_func)();
typedef struct scheduler_routine {
    char routine_name[MAX_ROUTINE_NAME_LEN + 1]; // routine name for logging purposes
    TickType_t execute_time; // exact tick when the routine will be ran
    TickType_t recur_time; // how many ticks until the routine will recur after execution, null for non-recurring routines
    routine_func routine_func_ptr; // function ptr that will run recurrently
} scheduler_routine;
typedef struct scheduler_context {
    SemaphoreHandle_t mutex; // mutex for thread-safety
    scheduler_routine* routines[MAX_ROUTINES]; // global storage for scheduler routines
    size_t routine_count; // amount of routines currently allocated
} scheduler_context;

// Global Scheduler Context
scheduler_context g_scheduler_context;

// Utility Functions
TickType_t ms_to_ticks(unsigned long ms);
unsigned long secs_to_ms(unsigned long secs);
unsigned long mins_to_secs(unsigned long mins);

// Recurring Routine Helper Functions
void schedule_recurring_task_ms(const char* routine_name, void* routine_func_ptr, unsigned int ms_until_recur);
void schedule_recurring_task_sec(const char* routine_name, void* routine_func_ptr, unsigned int ss_until_recur);
void schedule_recurring_task_mins(const char* routine_name, void* routine_func_ptr, unsigned int ss_until_recur);

// Delayed Routine Helper Functions
void schedule_delayed_task_ms(const char* routine_name, void* routine_func_ptr, unsigned int ms_delay);
void schedule_delayed_task_sec(const char* routine_name, void* routine_func_ptr, unsigned int ss_delay);
void schedule_delayed_task_mins(const char* routine_name, void* routine_func_ptr, unsigned int ss_delay);

// Internal Scheduler Functions
void create_scheduler_routine(const char* routine_name, TickType_t execute_time, TickType_t recur_time, routine_func routine_func_ptr);
bool run_scheduler_routine(scheduler_routine* routine);
void delete_scheduler_routine(scheduler_routine* routine);

// Main Task
void scheduler_task(void* unused_arg);

#endif /* !SCHEDULER_FILE_DEFINED */