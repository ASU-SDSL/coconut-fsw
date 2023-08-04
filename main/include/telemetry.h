#ifndef TELEMETRY_FILE_DEFINED
#define TELEMETRY_FILE_DEFINED

#include <stdio.h>
#include <string.h>

#include <FreeRTOS.h>
#include <task.h>
#include <semphr.h>
#include "pico/stdlib.h"
#include "gse.h"

#define TELEMETRY_MAX_QUEUE_ITEMS 64
#define TELEMETRY_CHECK_DELAY_MS 500

QueueHandle_t telemetry_queue;

/* USER FUNCTIONS */
void send_telemetry(const char* buffer, size_t size);

/* INTERNAL FUNCTIONS */
// Main Task
void telemetry_task(void* unused_arg);

#endif /* TELEMETRY_FILE_DEFINED */