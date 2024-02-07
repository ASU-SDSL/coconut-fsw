#include "steve.h"

/* 
    S.T.E.V.E.
    Scheduler Task for Executing Vital Events
*/

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

// Recurring Job Helper Functions
// These jobs will wait a specified time, run, wait a specified time, run, etc.
void schedule_recurring_job_ms(const char* job_name, job_func job_func_ptr, unsigned long ms_until_recur) {
    // Convert ms to ticks
    TickType_t recur_time = ms_to_ticks(ms_until_recur);
    // Calculate first run time 
    TickType_t execute_time = xTaskGetTickCount() + recur_time;
    // Create job
    queue_steve_job_creation(job_name, execute_time, recur_time, job_func_ptr);
}

void schedule_recurring_job_secs(const char* job_name, job_func job_func_ptr, unsigned long secs_until_recur) {
    schedule_recurring_job_ms(job_name, job_func_ptr, secs_to_ms(secs_until_recur));
}

void schedule_recurring_job_mins(const char* job_name, job_func job_func_ptr, unsigned long mins_until_recur) {
    schedule_recurring_job_secs(job_name, job_func_ptr, mins_to_secs(mins_until_recur));
}

// Delayed Job Helper Functions
// These tasks will wait a specified time, run, then never run again
void schedule_delayed_job_ms(const char* job_name, job_func job_func_ptr, unsigned long ms_delay) {
    // Convert ms to ticks
    TickType_t tick_delay = ms_to_ticks(ms_delay);
    // Calculate first run time 
    TickType_t execute_time = xTaskGetTickCount() + tick_delay;
    // Create job
    queue_steve_job_creation(job_name, execute_time, 0, job_func_ptr);
}

void schedule_delayed_job_secs(const char* job_name, job_func job_func_ptr, unsigned long secs_delay) {
    schedule_delayed_job_ms(job_name, job_func_ptr, secs_to_ms(secs_delay));
}

void schedule_delayed_job_mins(const char* job_name, job_func job_func_ptr, unsigned long mins_delay) {
    schedule_delayed_job_secs(job_name, job_func_ptr, mins_to_secs(mins_delay));
}

// Internal Scheduler Functions
void queue_steve_job_creation(const char* job_name, TickType_t execute_time, TickType_t recur_time, job_func job_func_ptr) {
    // Allocate memory
    steve_job_t* sr = pvPortMalloc(sizeof(steve_job_t));
    // Check name and copy
    size_t job_name_len = strlen(job_name);
    if (job_name_len + 1 > MAX_JOB_NAME_LEN) {
        logln_error("Name of job %s is too large, reduce to less than %u chars!!!", job_name, MAX_JOB_NAME_LEN);
        return;
    }
    strncpy(sr->name, job_name, job_name_len);
    // Copy other fields
    sr->execute_time = execute_time;
    sr->recur_time = recur_time;
    sr->func_ptr = job_func_ptr;
    while (!job_creation_queue) {
        vTaskDelay(ms_to_ticks(SCHEDULER_CHECK_DELAY_MS));
    }
    xQueueSendToBack(job_creation_queue, &sr, portMAX_DELAY);
}

void create_steve_job(steve_job_t* sr) {
    // Add job to list
    g_steve_context.jobs[g_steve_context.job_count++] = sr;
}

bool run_steve_job(steve_job_t* job) {
    // Run the job function
    job->func_ptr(job->arg_data);
    // Check if it needs to be rescheduled (recurring job)
    if (job->recur_time > 0) {
        job->execute_time = xTaskGetTickCount() + job->recur_time;
        return false; // returns false, the job list does not need a cleanup
    }
    // Otherwise delete the job
    else {
        delete_steve_job(job);
        return true; // returns true, the job list needs a cleanup
    }
}

void delete_steve_job(steve_job_t* job) {
    // TODO: Do more testing on this and the cleanup code
    // Find job in list and set job ptr to null to prevent UAF's
    bool found_ptr = false;
    for (int i = 0; i < g_steve_context.job_count; i++) {
        if (g_steve_context.jobs[i] == NULL) {
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
}

void cleanup_steve_jobs_list() {
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

void heartbeat_telemetry_job(void* unused) {
    // Create heartbeat struct
    heartbeat_telemetry_t payload;
    payload.state = g_payload_state;
    payload.uptime = get_uptime();

    // eps stuff
    i2c_inst_t *i2c = i2c0;
    uint16_t buf;
    getVShunt_raw(i2c, &buf);
    payload.eps_shunt = buf;
    getVBus_raw(i2c, &buf);
    payload.eps_vbus = buf;
    getCurrent_raw(i2c, &buf);
    payload.eps_current = buf; 
    getPower_raw(i2c, &buf);
    payload.eps_power = buf;

    // mag stuff 
    payload.mag_x = get_x_output(i2c);
    payload.mag_y = get_y_output(i2c);
    payload.mag_z = get_z_output(i2c);
    payload.mag_temp = get_temp_output(i2c);

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

}

TickType_t get_uptime() {
    // TODO: Maybe get an RTC instead of using CPU ticks
    return xTaskGetTickCount();
}

void initialize_steve() {
    // Initialize telemetry queue
    job_creation_queue = xQueueCreate(JOB_CREATION_MAX_QUEUE_ITEMS, sizeof(steve_job_t*));
    // Initialize state
    g_payload_state = INIT;
    // Create jobs
    schedule_recurring_job_secs(HEARTBEAT_JOB_NAME, heartbeat_telemetry_job, HEARTBEAT_TELEMETRY_DEFAULT_INTERVAL);
}

// Main thread job for scheduling and executing STEVE jobs
void steve_task(void* unused_arg) {
    // Setup STEVE jobs and state
    initialize_steve();
    // Run main task loop
    while (true) {
        // Check if we have any jobs to schedule
        steve_job_t* sd;
        if (xQueueReceive(job_creation_queue, &sd, 0)) {
            create_steve_job(sd);
        }
        // Check each job to see if it needs to be executed
        bool needs_cleanup = false;
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
                needs_cleanup |= run_steve_job(indexed_job);
            }
        }
        // Cleanup job list if neccessary
        if (needs_cleanup) {
            cleanup_steve_jobs_list();
        }
        // Sleep for a bit before checking again
        vTaskDelay(ms_to_ticks(SCHEDULER_CHECK_DELAY_MS));
    }
}