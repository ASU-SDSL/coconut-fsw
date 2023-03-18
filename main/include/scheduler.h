#include <stdio.h>

#include <FreeRTOS.h>
#include <task.h>
#include "pico/stdlib.h"

// #define SCHEDULER_CHECK_DELAY_MS 5000
#define MS_IN_SECS 1000
#define SECS_IN_MIN 60

// Recurring Routines
typedef void (*recurring_routine_func)();

struct recurring_routine_params {
    recurring_routine_func recurring_routine_func_ptr; // function ptr that will run recurrently
    TickType_t ticks_until_recur;      // how many ticks until the routine will recur
};

void recurring_task_routine(struct recurring_routine_params rrp);
void schedule_recurring_task_ms(const char* routine_name, void* routine_func_ptr, unsigned int ms_until_recur);
void schedule_recurring_task_sec(const char* routine_name, void* routine_func_ptr, unsigned int ss_until_recur);
void schedule_recurring_task_mins(const char* routine_name, void* routine_func_ptr, unsigned int ss_until_recur);

// Delayed Routines
typedef void (*delayed_routine_func)();

struct delayed_routine_params {
    delayed_routine_func delayed_routine_func_ptr;  // function ptr that will run recurrently
    TickType_t ticks_delay;                   // how many ticks until the routine will recur
};

void delayed_task_routine(struct delayed_routine_params rrp);
void schedule_delayed_task_ms(const char* routine_name, void* routine_func_ptr, unsigned int ms_delay);
void schedule_delayed_task_sec(const char* routine_name, void* routine_func_ptr, unsigned int ss_delay);
void schedule_delayed_task_mins(const char* routine_name, void* routine_func_ptr, unsigned int ss_delay);

// void scheduler_task(void* unused_arg);