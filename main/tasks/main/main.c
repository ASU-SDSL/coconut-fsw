#include "main.h"

/*
 * FREERTOS RUNTIME START
 */

#define GSE_ENABLED

int main() {

    //test comment

    // Initialize debug logging
    init_debug_log();

    // Print cool banner
    print_banner();
    
    // Set up the tasks/threads
    // Arg 3 is the stack depth -- in words, not bytes
    // Arg 5+ are arguments to the function of the task
    logln_info("Initializing FreeRTOS Tasks...");

    SemaphoreHandle_t sdMutex = xSemaphoreCreateMutex();

    if (sdMutex == NULL) {
        logln_error("Could not create SD mutex");
        while (true);
    }

#ifdef GSE_ENABLED
    BaseType_t gse_task_status = xTaskCreate(gse_task, 
                                        "GSE", 
                                        128, 
                                        &sdMutex,
                                        1,
                                        NULL);
#endif                       
    BaseType_t scheduler_task_status = xTaskCreate(steve_task, 
                                        "STEVE", 
                                        512, 
                                        NULL, 
                                        1,
                                        NULL); 

    BaseType_t command_task_status = xTaskCreate(command_task,
                                        "COMMAND",
                                        128,
                                        NULL,
                                        1,
                                        NULL);
    
    BaseType_t telemetry_task_status = xTaskCreate(telemetry_task,
                                        "TELEMETRY",
                                        128,
                                        NULL,
                                        1,
                                        NULL);
// #ifndef GSE_ENABLED
    BaseType_t radio_task_status = xTaskCreate(radio_task, 
                                         "RADIO", 
                                         128, 
                                         NULL, 
                                         1,
                                         NULL);
// #endif
    // Start the FreeRTOS scheduler
    vTaskStartScheduler();
    
    // We should never get here, but just in case...
    while(true){};
}
