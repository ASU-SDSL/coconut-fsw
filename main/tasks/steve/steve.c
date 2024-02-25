#include "steve.h"

/* 
    S.T.E.V.E.
    Scheduler Task for Executing Vital Events
*/

// Utility Functions
TickType_t ms_to_ticks(unsigned long ms) {
    return ms / portTICK_PERIOD_MS;
}

unsigned long ticks_to_ms(TickType_t ticks) {
    return ticks * portTICK_PERIOD_MS;
}

unsigned long secs_to_ms(unsigned long secs) {
    return secs * MS_IN_SEC;
}

unsigned long ms_to_secs(unsigned long ms) {
    return ms / MS_IN_SEC;
}

unsigned long secs_to_mins(unsigned long secs) {
    return secs / SECS_IN_MIN;
}

unsigned long mins_to_secs(unsigned long mins) {
    return mins * SECS_IN_MIN;
}

// Recurring Job User Functions
// These jobs will wait a specified time, run, wait a specified time, run, etc.
void schedule_recurring_job_ms(const char* job_name, job_func job_func_ptr, unsigned long ms_until_recur) {
    // Convert ms to ticks
    TickType_t recur_time = ms_to_ticks(ms_until_recur);
    // Calculate first run time 
    TickType_t execute_time = xTaskGetTickCount() + recur_time;
    // Create job
    create_steve_job(job_name, execute_time, recur_time, job_func_ptr);
}

void schedule_recurring_job_secs(const char* job_name, job_func job_func_ptr, unsigned long secs_until_recur) {
    schedule_recurring_job_ms(job_name, job_func_ptr, secs_to_ms(secs_until_recur));
}

void schedule_recurring_job_mins(const char* job_name, job_func job_func_ptr, unsigned long mins_until_recur) {
    schedule_recurring_job_secs(job_name, job_func_ptr, mins_to_secs(mins_until_recur));
}

// Delayed Job User Functions
// These tasks will wait a specified time, run, then never run again
void schedule_delayed_job_ms(const char* job_name, job_func job_func_ptr, unsigned long ms_delay) {
    // Convert ms to ticks
    TickType_t tick_delay = ms_to_ticks(ms_delay);
    // Calculate first run time 
    TickType_t execute_time = xTaskGetTickCount() + tick_delay;
    // Create job
    create_steve_job(job_name, execute_time, 0, job_func_ptr);
}

void schedule_delayed_job_secs(const char* job_name, job_func job_func_ptr, unsigned long secs_delay) {
    schedule_delayed_job_ms(job_name, job_func_ptr, secs_to_ms(secs_delay));
}

void schedule_delayed_job_mins(const char* job_name, job_func job_func_ptr, unsigned long mins_delay) {
    schedule_delayed_job_secs(job_name, job_func_ptr, mins_to_secs(mins_delay));
}

// Job Management User Functions

void kill_steve_job(const char* job_name) {
    // Take mutex
    xSemaphoreTake(g_steve_context.mutex, portMAX_DELAY);
    // Find job
    steve_job_t* job = find_steve_job(job_name);
    // Delete job
    delete_steve_job(job);
    // Give mutex back
    xSemaphoreGive(g_steve_context.mutex);
}

void edit_steve_job_recur_time(const char* job_name, unsigned long ms_recur_time) {
    // logln_info("Changing %s job recur time to: %d ms", job_name, ms_recur_time);
    // Take mutex
    xSemaphoreTake(g_steve_context.mutex, portMAX_DELAY);
    // Find job
    steve_job_t* job = find_steve_job(job_name);
    if (job == NULL) {
        logln_error("Can't find task to change recur time! %s", job_name)
        return;
    }
    // Roll back execution time
    job->execute_time -= job->recur_time;
    // Edit recur time
    job->recur_time = ms_recur_time;
    // Set new execution time
    job->execute_time += job->recur_time;
    // Give mutex back
    xSemaphoreGive(g_steve_context.mutex);
}

void print_debug_exec_times() {
    logln_info("Printing S.T.E.V.E Task Times:");
    // Take mutex
    xSemaphoreTake(g_steve_context.mutex, portMAX_DELAY);
    for (int i = 0; i < g_steve_context.job_count; i++) {
        steve_job_t *temp = g_steve_context.jobs[i];
        // make sure job is valid
        if (temp == NULL) {
            continue;
        }
        // print details
        int secs_until_exec = ms_to_secs(xTaskGetTickCount() - temp->execute_time);
        logln_info("%s executing in %d seconds", temp->name, secs_until_exec);
    }
    // Give mutex back
    xSemaphoreGive(g_steve_context.mutex);
}

// Internal Scheduler Functions

steve_job_t* find_steve_job(const char* job_name) {
    /* g_steve_context.mutex must be taken before using this function */
    for (int i = 0; i < g_steve_context.job_count; i++) {
        steve_job_t *temp = g_steve_context.jobs[i];
        // make sure job is valid
        if (temp == NULL) {
            continue;
        }
        // check name
        if (strncmp(job_name, temp->name, MAX_JOB_NAME_LEN) == 0) {
            return temp;
        }
    }
    return NULL;
}

void create_steve_job(const char* job_name, TickType_t execute_time, TickType_t recur_time, job_func job_func_ptr) {
    // Checks
    if (!g_steve_context.mutex) {
        logln_error("You can only create tasks after initialize_steve() is called!!!");
        return;
    }
    xSemaphoreTake(g_steve_context.mutex, portMAX_DELAY);
    if (g_steve_context.job_count >= (MAX_JOBS-1)) {
        logln_error("Too many jobs!!! You can only create %d jobs", MAX_JOBS);
        return;
    }
    xSemaphoreGive(g_steve_context.mutex);
    // Check name
    size_t job_name_len = strlen(job_name);
    if (job_name_len + 1 > MAX_JOB_NAME_LEN) {
        logln_error("Name of job %s is too large, reduce to less than %u chars!!!", job_name, MAX_JOB_NAME_LEN);
        return;
    }
    // Allocate memory
    steve_job_t* sr = pvPortMalloc(sizeof(steve_job_t));
    // Copy name
    strncpy(sr->name, job_name, job_name_len + 1);
    // Copy other fields
    sr->execute_time = execute_time;
    sr->recur_time = recur_time;
    sr->func_ptr = job_func_ptr;
    // Add job to list
    xSemaphoreTake(g_steve_context.mutex, portMAX_DELAY);
    g_steve_context.jobs[g_steve_context.job_count++] = sr;
    xSemaphoreGive(g_steve_context.mutex);
}

void run_steve_job(steve_job_t* job) {
    /* g_steve_context.mutex must be taken before using this function */
    // Run the job function
    job->func_ptr(job->arg_data);
    // Check if it needs to be rescheduled (recurring job)
    if (job->recur_time > 0) {
        job->execute_time = xTaskGetTickCount() + job->recur_time;
    }
    // Otherwise delete the job
    else {
        delete_steve_job(job);
    }
}

void delete_steve_job(steve_job_t* job) {
    /* g_steve_context.mutex must be taken before using this function */
    // Find job in list and set job ptr to null to prevent UAF's
    bool found_ptr = false;
    for (int i = 0; i < g_steve_context.job_count; i++) {
        steve_job_t *temp = g_steve_context.jobs[i];
        if (temp == job) {
            found_ptr = true;
            g_steve_context.jobs[i] = NULL;
            break;
        }
    }
    // If it couldn't find the pointer, something is wrong
    if (!found_ptr) {
        logln_error("Couldn't find scheduler job to delete: %s", job->name);
        return;
    }
    // Free the struct memory
    vPortFree(job);
    // Cleanup job list
    cleanup_steve_jobs_list();
}

void cleanup_steve_jobs_list() {
    /* g_steve_context.mutex must be taken before using this function */
    // Find null entries in the scheduler list
    for (int i = 0; i < g_steve_context.job_count; i++) {
        if (g_steve_context.jobs[i] == NULL) {
            // Swap the null job with the last job in the list
            g_steve_context.jobs[i] = g_steve_context.jobs[g_steve_context.job_count - 1];
            g_steve_context.jobs[g_steve_context.job_count - 1] = NULL;
            // Decrement job size counter
            g_steve_context.job_count -= 1;
        }
    }
}

TickType_t get_uptime() {
    // TODO: Maybe get an RTC instead of using CPU ticks
    return xTaskGetTickCount();
}

uint32_t counter_lol;

void heartbeat_telemetry_job(void* unused) {
    // Create heartbeat struct
    heartbeat_telemetry_t payload;
    payload.state = g_payload_state;
    payload.uptime = get_uptime();
    // Send it
    send_telemetry(HEARTBEAT, (char*)&payload, sizeof(payload));
    //logln_info("Lol");

    // get mag info
    //i2c_inst_t *i2c = i2c0;
    //int x = get_x_output(i2c);

    /*
    char str[10];
    sprintf(str, "X: %d", x);
    logln_info(str);*/
    counter_lol += 1;
    logln_info("Loop %ld\n", counter_lol);
}

void initialize_steve() {
    // Initialize scheduler job mutex
    g_steve_context.mutex = xSemaphoreCreateMutex();
    // Initialize state
    g_payload_state = INIT;
    // Create jobs
    counter_lol = 0;
    schedule_recurring_job_secs(HEARTBEAT_JOB_NAME, heartbeat_telemetry_job, HEARTBEAT_TELEMETRY_DEFAULT_INTERVAL);
}

// Main thread job for scheduling and executing STEVE jobs
void steve_task(void* unused_arg) {
    logln_info("Test");
    // Setup STEVE jobs and state
    initialize_steve();
    // Run main task loop
    while (true) {
        // Take mutex
        xSemaphoreTake(g_steve_context.mutex, portMAX_DELAY);
        // Check each job to see if it needs to be executed
        for (int i = 0; i < g_steve_context.job_count; i++) {
            // Get indexed job
            steve_job_t* indexed_job = g_steve_context.jobs[i];
            // Check if job is valid
            if (indexed_job == NULL) {
                continue;
            }
            // Check if job is ready to run
            if (indexed_job->execute_time >= xTaskGetTickCount()) {
                // Run the job if so
                run_steve_job(indexed_job);
            }
        }
        // Give back mutex
        xSemaphoreGive(g_steve_context.mutex);
        // Sleep for a bit before checking again
        vTaskDelay(ms_to_ticks(SCHEDULER_CHECK_DELAY_MS));
    }
}