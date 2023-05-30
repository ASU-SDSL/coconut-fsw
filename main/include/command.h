#ifndef COMMAND_FILE_DEFINED
#define COMMAND_FILE_DEFINED

#include <stdio.h>
#include <string.h>

#include <FreeRTOS.h>
#include <task.h>
#include <semphr.h>
#include "pico/stdlib.h"
#include "queue.h"
#include "gse.h"

#define COMMAND_MAX_QUEUE_ITEMS 64
#define COMMAND_CHECK_DELAY_MS 500

QueueHandle_t command_byte_queue;

typedef enum command_source {
    UART,
    RADIO
} command_source_t;

typedef struct command_byte {
    command_source_t source;
    char value;
} command_byte_t;

/* USER FUNCTIONS */
void receive_command_byte_from_isr(char ch, command_source_t source);

/* INTERNAL FUNCTIONS */

// Main Task
void command_task(void* unused_arg);

#endif /* !COMMAND_FILE_DEFINED */