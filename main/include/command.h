#ifndef COMMAND_FILE_DEFINED
#define COMMAND_FILE_DEFINED

#include <stdio.h>
#include <string.h>

#include <FreeRTOS.h>
#include <task.h>
#include <semphr.h>
#include "pico/stdlib.h"
#include "gse.h"

#define COMMAND_MAX_QUEUE_ITEMS 64

typedef char* command_message_type;
QueueHandle_t command_queue;

void parse_inbound_command(command_message_type command);

// Main Task
void command_task(void* unused_arg);

#endif /* !COMMAND_FILE_DEFINED */