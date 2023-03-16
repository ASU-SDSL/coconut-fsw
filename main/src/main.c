#include "main.h"
#include "gse.h"
#include "radio.h"
#include "missionmanager.h"

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
    stdio_init_all();   // I was not able to read anything out from the serial port
    
    // Set up the tasks/threads
    // NOTE Arg 3 is the stack depth -- in words, not bytes
    BaseType_t missionmanager_task_status = xTaskCreate(missionmanager_task, 
                                         "PICO_LED_TASK", 
                                         128, 
                                         NULL, 
                                         1,
                                         NULL); // this last parameter is the parameter to the function of the task
    BaseType_t gse_task_status = xTaskCreate(gse_task, 
                                         "GPIO_LED_TASK", 
                                         128, 
                                         NULL, 
                                         1,
                                         NULL);
    BaseType_t radio_task_status = xTaskCreate(radio_task, 
                                         "GPIO_LED_TASK", 
                                         128, 
                                         NULL, 
                                         1,
                                         NULL);
    
    // Log app info
    log_device_info();
    
    // Start the FreeRTOS scheduler
    // FROM 1.0.1: Only proceed with valid tasks
    if (missionmanager_task_status == pdPASS || gse_task_status == pdPASS || radio_task_status == pdPASS) {
        vTaskStartScheduler();
    }
    
    // We should never get here, but just in case...
    while(true) {
        // NOP
    };
}
