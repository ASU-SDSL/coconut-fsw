/* Includes and Constants */
#pragma once

#include "FreeRTOS.h"
#include "queue.h"

#include <semphr.h>
#include "ff.h"

#include "log.h"

#define READ_BUFFER_SIZE 256
#define FILESYSTEM_QUEUE_LENGTH 64
#define EMPTY_QUEUE_WAIT_TIME portMAX_DELAY
#define NULL_QUEUE_WAIT_TIME 300
#define MRAM_DRIVE_NUMBER 0

/* Types and Globals */
typedef enum fs_operation_type {
    MAKE_FILESYSTEM,
    READ,
    WRITE
} fs_operation_type_t;

typedef struct filesystem_queue_operations {
    fs_operation_type_t operation_type;
    const char* file_name;
    char* text_to_write;
    char* read_buffer;
    size_t size;
} filesystem_queue_operations_t;

typedef enum mounting_options {
    MOUNT_ON_FIRST_ACCESS,
    FORCE_MOUNT
} mounting_options_t;

QueueHandle_t filesystem_queue;

/* User Functions */
void make_filesystem();
void read_file(const char* file_name, char* result_buffer);
void write_file(const char* file_name, char* text_to_write);

/* Internal Functions */
size_t _mkfs();
size_t _read(const char *file_name, char* result_buffer, size_t size);
size_t _write(const char *file_name, const uint8_t *data, bool append_flag, size_t size);
void _delete(const char *file_name);
void _list(const char *dir_name);
void _test();

// Main Task
void filesystem_task(void* unused_arg);