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

QueueHandle_t command_queue;

void receive_byte_from_transmission(char byte);

void queue_inbound_command(transmission_buffer command);

// Main Task
void command_task(void* unused_arg);

#endif /* !COMMAND_FILE_DEFINED */