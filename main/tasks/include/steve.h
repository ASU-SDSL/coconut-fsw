#pragma once

#include <stdio.h>
#include <string.h>
#include <stdbool.h>

#include <FreeRTOS.h>
#include <task.h>
#include <semphr.h>
#include "pico/stdlib.h"

#include "telemetry.h"
#include "log.h"
#include "state.h"
#include "mag.h"

// Constants
#define MAX_JOBS 256
#define MAX_JOB_NAME_LEN 32
#define SCHEDULER_CHECK_DELAY_MS 500
#define MS_IN_SEC 1000
#define SECS_IN_MIN 60
#define JOB_CREATION_MAX_QUEUE_ITEMS 16

// Job Structs and Types
typedef void (*job_func)(void*);

typedef struct steve_job {
    char name[MAX_JOB_NAME_LEN + 1]; // job name for logging purposes
    TickType_t execute_time; // exact tick when the job will be ran
    TickType_t recur_time; // how many ticks until the job will recur after execution, null for non-recurring jobs
    job_func func_ptr; // function ptr that will be called when the job is ran
    void* arg_data; // data that will be passed to the first argument of the function job
} steve_job_t;

typedef struct steve_context {
    steve_job_t* jobs[MAX_JOBS]; // global storage for scheduler jobs
    size_t job_count; // amount of jobs currently allocated
} steve_context_t;

// Global Scheduler Context
steve_context_t g_steve_context;
QueueHandle_t job_creation_queue;

/* USER FUNCTIONS */

// Utility Functions
TickType_t ms_to_ticks(unsigned long ms);
unsigned long secs_to_ms(unsigned long secs);
unsigned long mins_to_secs(unsigned long mins);

// Recurring Job Helper Functions
void schedule_recurring_job_ms(const char* job_name, job_func job_func_ptr, unsigned long ms_until_recur);
void schedule_recurring_job_secs(const char* job_name, job_func job_func_ptr, unsigned long secs_until_recur);
void schedule_recurring_job_mins(const char* job_name, job_func job_func_ptr, unsigned long mins_until_recur);

// Delayed Job Helper Functions
void schedule_delayed_job_ms(const char* job_name, job_func job_func_ptr, unsigned long ms_delay);
void schedule_delayed_job_secs(const char* job_name, job_func job_func_ptr, unsigned long secs_delay);
void schedule_delayed_job_mins(const char* job_name, job_func job_func_ptr, unsigned long mins_delay);

/* INTERNAL FUNCTIONS */

// Defined Jobs
#define HEARTBEAT_JOB_NAME "HEARTBEAT_TELEMETRY"
#define HEARTBEAT_TELEMETRY_DEFAULT_INTERVAL 5
void heartbeat_telemetry_job(void*);

// Helper functions
TickType_t get_uptime();

// Internal Scheduler Functions
void queue_steve_job_creation(const char* job_name, TickType_t execute_time, TickType_t recur_time, job_func job_func_ptr);
void create_steve_job(steve_job_t* sr);
bool run_steve_job(steve_job_t* job);
void delete_steve_job(steve_job_t* job);
void cleanup_steve_jobs_list();
void initialize_steve();

// Main Task
void steve_task(void* unused_arg);