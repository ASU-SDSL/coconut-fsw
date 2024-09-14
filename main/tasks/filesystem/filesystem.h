/*
TODO:   sd card driver does not look finished, so finish the driver to read and write to sd card, plus any other qol functions
        write user functions. all internal functions and structs will come out of necessity.

*/

/* Includes and Constants */
#pragma once

#include "storage.h"
#include <semphr.h>
#include "queue.h"
#include "FreeRTOS.h"
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

//write file
// may want to add return value to specify whether operation was successful
void write_file(const char* file_name, char* text_to_write);

//append file
void append_file(const char* file_name, const char* text_to_append);

//create file
void create_file(const char* new_file_name);

//delete file
void delete_file(const char* file_name);


/* Internal Functions */


// Main Task
void sd_task(void* unused_arg);