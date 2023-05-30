#ifndef SCHEDULER_FILE_DEFINED
#define SCHEDULER_FILE_DEFINED

#include <stdio.h>
#include <string.h>

#include <FreeRTOS.h>
#include <task.h>
#include <semphr.h>
#include "pico/stdlib.h"

#include "telemetry.h"

// Constants
#define MAX_ROUTINES 256
#define MAX_ROUTINE_NAME_LEN 100
#define SCHEDULER_CHECK_DELAY_MS 500
#define MS_IN_SEC 1000
#define SECS_IN_MIN 60
#define ROUTINE_CREATION_MAX_QUEUE_ITEMS 16

// Structs and Types
typedef void (*routine_func)(void*);
typedef struct scheduler_routine {
    char name[MAX_ROUTINE_NAME_LEN + 1]; // routine name for logging purposes
    TickType_t execute_time; // exact tick when the routine will be ran
    TickType_t recur_time; // how many ticks until the routine will recur after execution, null for non-recurring routines
    routine_func func_ptr; // function ptr that will be called when the routine is ran
    void* arg_data; // data that will be passed to the first argument of the function routine
} scheduler_routine;
typedef struct scheduler_context {
    scheduler_routine* routines[MAX_ROUTINES]; // global storage for scheduler routines
    size_t routine_count; // amount of routines currently allocated
} scheduler_context;

// Global Scheduler Context
scheduler_context g_scheduler_context;
QueueHandle_t routine_creation_queue;

/* USER FUNCTIONS */

// Utility Functions
TickType_t ms_to_ticks(unsigned long ms);
unsigned long secs_to_ms(unsigned long secs);
unsigned long mins_to_secs(unsigned long mins);

// Recurring Routine Helper Functions
void schedule_recurring_routine_ms(const char* routine_name, routine_func routine_func_ptr, unsigned long ms_until_recur);
void schedule_recurring_routine_secs(const char* routine_name, routine_func routine_func_ptr, unsigned long secs_until_recur);
void schedule_recurring_routine_mins(const char* routine_name, routine_func routine_func_ptr, unsigned long mins_until_recur);

// Delayed Routine Helper Functions
void schedule_delayed_routine_ms(const char* routine_name, routine_func routine_func_ptr, unsigned long ms_delay);
void schedule_delayed_routine_secs(const char* routine_name, routine_func routine_func_ptr, unsigned long secs_delay);
void schedule_delayed_routine_mins(const char* routine_name, routine_func routine_func_ptr, unsigned long mins_delay);

/* INTERNAL FUNCTIONS */

// Internal Scheduler Functions
void queue_scheduler_routine_creation(const char* routine_name, TickType_t execute_time, TickType_t recur_time, routine_func routine_func_ptr);
void create_scheduler_routine(scheduler_routine* sr);
bool run_scheduler_routine(scheduler_routine* routine);
void delete_scheduler_routine(scheduler_routine* routine);
void cleanup_scheduler_routines_list();
void initialize_scheduler_context();

// Main Task
void scheduler_task(void* unused_arg);

#endif /* !SCHEDULER_FILE_DEFINED */