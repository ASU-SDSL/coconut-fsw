#pragma once

#include "FreeRTOS.h"
#include "queue.h"

#include <semphr.h>
#include "ff.h"

#include "log.h"

#define FILESYSTEM_QUEUE_LENGTH 64
#define EMPTY_QUEUE_WAIT_TIME portMAX_DELAY
#define NULL_QUEUE_WAIT_TIME 300

/* Types and Globals */

// enum to define different file system operations
typedef enum fs_operation_type {
    WRITE,
    READ
} fs_operation_type_t;

// struct to represent operations passed to the queue
// TODO: come up with actual struct def
typedef struct filesystem_queue_operations {
    fs_operation_type_t operation_type;
    const char* file_name;
    char* text_to_write;
    char* read_buffer;
} filesystem_queue_operations_t;



// queue to accept incoming operations on the file system
QueueHandle_t filesystem_queue;

/* User Functions */

//read file
void read_file(const char* file_name, char* result_buffer);

// write file
// may want to add return value to specify whether operation was successful
void write_file(const char* file_name, char* text_to_write);

//append file
void append_file(const char* file_name, const char* text_to_append);

//create file
void create_file(const char* new_file_name);

//delete file
void delete_file(const char* file_name);


/* Internal Functions */

size_t _write(const char *file_name, const uint8_t *data, bool append_flag, size_t size);
size_t _read(const char *file_name, char* result_buffer, size_t size);
void _delete(const char *file_name);

// Main Task
void filesystem_task(void* unused_arg);