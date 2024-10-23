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
#define NULL_QUEUE_WAIT_TIME 100
#define MRAM_DRIVE_NUMBER 0
#define CAT_SIZE_LIMIT 128

#define MAX_PATH_SIZE 0x100
#define MAX_WRITE_CONTENTS_SIZE 0x100

/* Types and Globals */
typedef enum fs_operation_type {
    MAKE_FILESYSTEM,
    READ,
    WRITE,
    LIST_DIRECTORY,
    DELETE,
    MAKE_DIRECTORY
} fs_operation_type_t;

typedef struct read_operation {
    const char file_name[MAX_PATH_SIZE];
    char* read_buffer;
    size_t size;
} read_operation_t;

typedef struct write_operation {
    const char file_name[MAX_PATH_SIZE];
    char text_to_write[MAX_WRITE_CONTENTS_SIZE];
    size_t size;
    bool append_flag;
} write_operation_t;

typedef struct list_directory_operation {
    const char directory_name[MAX_PATH_SIZE];
} list_directory_operation_t;

typedef struct delete_operation {
    const char file_name[MAX_PATH_SIZE];
} delete_operation_t;

typedef struct make_directory_operation {
    const char* directory_name[MAX_PATH_SIZE];
} make_directory_operation_t;

typedef union file_operation_union{
    read_operation_t read_op;
    write_operation_t write_op;
    list_directory_operation_t ls_op;
    delete_operation_t delete_op;
    make_directory_operation_t mkdir_op;
} file_operation_union_t;

typedef struct filesystem_queue_operations {
    fs_operation_type_t operation_type;
    file_operation_union_t file_operation;
} filesystem_queue_operations_t;

typedef enum mounting_options {
    MOUNT_ON_FIRST_ACCESS,
    FORCE_MOUNT
} mounting_options_t;

SemaphoreHandle_t filesystem_mutex;

QueueHandle_t filesystem_queue;

/* User Functions */
void make_filesystem();
void read_file(const char* file_name, char* result_buffer, size_t size);
void write_file(const char* file_name, char* text_to_write, size_t size, bool append_flag);
void list_directory(const char* directory_name);
void delete_file(const char* file_name);
void make_directory(const char* directory_name);
void touch(const char* file_name);
void cat(const char* file_name);

/* Internal Functions */
size_t _mkfs();
size_t _fread(const char *file_name, char* result_buffer, size_t size);
size_t _fwrite(const char *file_name, const uint8_t *data, size_t size, bool append_flag);
void _fdelete(const char *file_name);
void _flist(const char *dir_name);
void _fmkdir(const char *dir_name);
void _test();

// Main Task
void filesystem_task(void* unused_arg);