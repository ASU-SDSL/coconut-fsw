#include "main.h"
#include "gse.h"
#include "radio.h"
#include "scheduler.h"
#include "command.h"

/*
 * GLOBALS
 */
// we may need to make globals (like queues, mutexs, etc) here and then pass them as arguments to the tasks

const TickType_t ms_delay = 1000 / portTICK_PERIOD_MS; // 500 ms


/**
 * @brief Generate and print a debug message from a supplied string.
 *
 * @param msg: The base message to which `[DEBUG]` will be prefixed.
 */
/*
void log_debug(const char* msg) {
    uint msg_length = 9 + strlen(msg);
    char* sprintf_buffer = malloc(msg_length);
    sprintf(sprintf_buffer, "[DEBUG] %s\n", msg);
    #ifdef DEBUG
    printf("%s", sprintf_buffer);
    #endif
    free(sprintf_buffer);
}*/


/**
 * @brief Show basic device info.
 */
void log_device_info(void) {
    printf("Project: %s", PROJECT_NAME);
}

/*
 * RUNTIME START
 */
int main() {

    // Enable STDIO
    stdio_init_all();
    
    // Set up the tasks/threads
    // Arg 3 is the stack depth -- in words, not bytes
    // Arg 5+ are arguments to the function of the task
    BaseType_t gse_task_status = xTaskCreate(gse_task, 
                                         "GSE_TASK", 
                                         128, 
                                         NULL, 
                                         1,
                                         NULL);
                                         
    BaseType_t scheduler_task_status = xTaskCreate(scheduler_task, 
                                         "SCHEDULER_TASK", 
                                         512, 
                                         NULL, 
                                         1,
                                         NULL); 

    BaseType_t command_task_status = xTaskCreate(command_task,
                                            "COMMAND_TASK",
                                            128,
                                            NULL,
                                            1,
                                            NULL);

    BaseType_t radio_task_status = xTaskCreate(radio_task, 
                                         "RADIO_TASK", 
                                         128, 
                                         NULL, 
                                         1,
                                         NULL);
    
    // Log app info
    log_device_info();
    
    // Start the FreeRTOS scheduler
    if (/*scheduler_task_status == pdPASS || */ command_task_status == pdPASS || gse_task_status == pdPASS || radio_task_status == pdPASS) {
        vTaskStartScheduler();
    }
    
    // We should never get here, but just in case...
    while(true){};
}
