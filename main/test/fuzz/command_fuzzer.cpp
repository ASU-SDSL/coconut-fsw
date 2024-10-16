
#include <stdbool.h>
#include <stdlib.h>
#include <stdint.h>
#include <string.h>
#include <time.h>
#include <sys/stat.h>


#include "FreeRTOS.h"
#include "queue.h"
#include "task.h"

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

extern "C" void fuzzer_task(void *input) {
  fuzzer_input_t *fuzzinput = (fuzzer_input_t *)input;
  // Send commands
  receive_command_bytes(fuzzinput->data, fuzzinput->size);
  // Wait
  vTaskDelay(10);
  // End process
  // vTaskEndScheduler();
  exit(0);
}

extern "C" int LLVMFuzzerTestOneInput(uint8_t *Data, uint32_t Size) {
  // Close stdout and stderr
  // close(1);
  // close(2);
  ////// Start tasks 

  TaskHandle_t scheduler_task_handle;
  BaseType_t scheduler_task_status = xTaskCreate(steve_task, 
                                      "STEVE", 
                                      512, 
                                      NULL, 
                                      1,
                                      &scheduler_task_handle); 

  TaskHandle_t command_task_handle;
  BaseType_t command_task_status = xTaskCreate(command_task,
                                      "COMMAND",
                                      512,
                                      NULL,
                                      1,
                                      &command_task_handle);
  
  TaskHandle_t telemetry_task_handle;
  BaseType_t telemetry_task_status = xTaskCreate(telemetry_task,
                                      "TELEMETRY",
                                      256,
                                      NULL,
                                      1,
                                      &telemetry_task_handle);

  TaskHandle_t filesystem_task_handle;
  BaseType_t filesystem_task_status = xTaskCreate(filesystem_task,
                                      "FILESYSTEM",
                                      512,
                                      NULL,
                                      1,
                                      &filesystem_task_handle);

  ////// Setup fuzzer thread
  fuzzer_input_t input;
  input.data = Data;
  input.size = Size;                 
  TaskHandle_t fuzzer_task_handle;
  BaseType_t fuzzer_task_status = xTaskCreate(fuzzer_task,
                                      "FUZZER",
                                      256,
                                      &input,
                                      1,
                                      &fuzzer_task_handle);

  ////// Start the FreeRTOS scheduler
  vTaskStartScheduler();
  return 0;
}

int main(int argc, char** argv) {
  char fopenstr[3] = {'r', 'b', '\x00'};
  FILE *fileptr = fopen(argv[1], fopenstr);  // Open the file in binary mode
  struct stat st;
  lstat(argv[1], &st);
  size_t filelen = st.st_size;
  void *buf = malloc(filelen);
  fread(buf, filelen, 1, fileptr); // Read in the entire file
  LLVMFuzzerTestOneInput((uint8_t *)buf, filelen);
  free(buf);
  return 0;
}
