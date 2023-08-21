#include "main.h"

/*
 * FREERTOS RUNTIME START
 */

int main() {

    //test comment

    // Initialize debug logging
    init_debug_log();

    // Print cool banner
    print_banner();
    
    // Set up the tasks/threads
    // Arg 3 is the stack depth -- in words, not bytes
    // Arg 5+ are arguments to the function of the task
    log_info("Initializing FreeRTOS Tasks...");
    BaseType_t gse_task_status = xTaskCreate(gse_task, 
                                        "GSE", 
                                        128, 
                                        NULL, 
                                        1,
                                        NULL);
                                         
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

    /*BaseType_t radio_task_status = xTaskCreate(radio_task, 
                                         "RADIO", 
                                         128, 
                                         NULL, 
                                         1,
                                         NULL);*/
    
    // Start the FreeRTOS scheduler
    if (/*scheduler_task_status == pdPASS || */ command_task_status == pdPASS || gse_task_status == pdPASS /*|| radio_task_status == pdPASS*/) {
        vTaskStartScheduler();
    }
    
    // We should never get here, but just in case...
    while(true){};
}
