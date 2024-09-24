#include <FreeRTOS.h>
#include <task.h>
#include <queue.h>

#include <stdbool.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <time.h>
#include "pico/stdlib.h"


extern "C" {
  BaseType_t xTaskCreate( TaskFunction_t pxTaskCode,
                              const char * const pcName,
                              const configSTACK_DEPTH_TYPE uxStackDepth,
                              void * const pvParameters,
                              UBaseType_t uxPriority,
                              TaskHandle_t * const pxCreatedTask );

  void gse_task(void *pvParameters);
}

#include "gse.h"
#include "command.h"
#include "telemetry.h"
#include "log.h"
#include "steve.h"
#include "filesystem.h"

extern "C" int LLVMFuzzerTestOneInput(const uint8_t *Data, uint32_t Size) {
  // Start tasks 
  BaseType_t gse_task_status = xTaskCreate(gse_task, 
                                      "GSE", 
                                      256, 
                                      NULL,
                                      1,
                                      NULL);            
  
  return 0;
}