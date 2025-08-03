#include <stdbool.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include "pico/stdlib.h"

#include <FreeRTOS.h>
#include <task.h>
#include <queue.h>


#include "gse.h"
#include "command.h"
#include "telemetry.h"
#include "log.h"
#include "steve.h"
#include "filesystem.h"
#include "watchdog.h"

#ifndef SIMULATOR
#include "radio.h"
#endif

#include "main.h"

/*
 * Coconut CubeSat Flight Software
 * ASU Sun Devil Satellite Lab
 */



int main() {
    
#if defined(DEBUG) && !defined(SIMULATOR)
    timer_hw->dbgpause = 0;
#endif


    BaseType_t gse_task_status = xTaskCreate(gse_task, 
                                        "GSE", 
                                        256, 
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
                                        1024,
                                        NULL,
                                        1,
                                        NULL);
    
    BaseType_t telemetry_task_status = xTaskCreate(telemetry_task,
                                        "TELEMETRY",
                                        1024,
                                        NULL,
                                        1,
                                        NULL);

#ifndef SIMULATOR
    BaseType_t radio_task_status = xTaskCreate(radio_task, 
                                         "RADIO", 
                                         256, 
                                         NULL, 
                                         1,
                                         NULL);
#endif


    BaseType_t filesystem_task_status = xTaskCreate(filesystem_task,
                                        "FILESYSTEM",
                                        1024,
                                        NULL,
                                        1,
                                        NULL);

    BaseType_t watchdog_task_status = xTaskCreate(watchdog_task,
                                        "WATCHDOG",
                                        1024,
                                        NULL,
                                        1,
                                        &xWatchdogTaskHandler);
                                        
    // Start the FreeRTOS scheduler
    vTaskStartScheduler();
    
    // We should never get here, but just in case...
    while(true){};
}
