#include <FreeRTOS.h>
#include <task.h>
#include <queue.h>

#include <stdbool.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <time.h>
#include "pico/stdlib.h"

#include "ccsds.h"

extern "C" {
  BaseType_t xTaskCreate( TaskFunction_t pxTaskCode,
                              const char * const pcName,
                              const configSTACK_DEPTH_TYPE uxStackDepth,
                              void * const pvParameters,
                              UBaseType_t uxPriority,
                              TaskHandle_t * const pxCreatedTask );

  void gse_task(void *pvParameters);

  void steve_task(void *pvParameters);
  
  void command_task(void *pvParameters);
  
  void telemetry_task(void *pvParameters);

  void filesystem_task(void *pvParameters);

  void receive_command_bytes(uint8_t* packet, size_t packet_size);
}

#include "gse.h"
#include "command.h"
#include "telemetry.h"
#include "log.h"
#include "steve.h"
#include "filesystem.h"

typedef struct fuzzer_input {
  uint8_t *data;
  size_t size;
} fuzzer_input_t;

extern "C" void fuzzer_task(fuzzer_input_t *input) {
  // Send commands
  receive_command_bytes(input->data, input->size);
  // End process
  vTaskEndScheduler();
}

extern "C" int LLVMFuzzerTestOneInput(const uint8_t *Data, uint32_t Size) {
  // Start tasks 
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
                                      512,
                                      NULL,
                                      1,
                                      NULL);
  
  BaseType_t telemetry_task_status = xTaskCreate(telemetry_task,
                                      "TELEMETRY",
                                      256,
                                      NULL,
                                      1,
                                      NULL);

  BaseType_t filesystem_task_status = xTaskCreate(filesystem_task,
                                      "FILESYSTEM",
                                      256,
                                      NULL,
                                      1,
                                      NULL);

  // Setup fuzzer thread
  fuzzer_input_t input;
  input.data = (uint8_t *)Data;
  input.size = Size;                 
  BaseType_t fuzzer_task_status = xTaskCreate((void (*)(void *))fuzzer_task,
                                      "FUZZER",
                                      4096,
                                      &input,
                                      1,
                                      NULL);
  // Start the FreeRTOS scheduler
  // vTaskStartScheduler();
  
  return 0;
}