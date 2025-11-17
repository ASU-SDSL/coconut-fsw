/**
 * @file filesystem.h
 * @brief Handles file system operations and management
 * 
 */

/* Includes and Constants */
#pragma once

#include "FreeRTOS.h"
#include "queue.h"
#include "task.h"
#include "ff.h"

#include <semphr.h>

#include <stddef.h>
#include <stdint.h>
#include <stdbool.h>

#define READ_BUFFER_SIZE 256
#define FILESYSTEM_QUEUE_LENGTH 64
#define EMPTY_QUEUE_WAIT_TIME portMAX_DELAY
#define NULL_QUEUE_WAIT_TIME 1
#define MRAM_DRIVE_NUMBER 0
#define CAT_SIZE_LIMIT 128
#define NOTIFICATION_WAIT_TIME pdMS_TO_TICKS(10 * 1000)

#define MAX_PATH_SIZE 128
#define MAX_WRITE_CONTENTS_SIZE 0x100

/* File storage allocation */
#define ERROR_LOGS_FS_ALLOCATION 0x300U // Max bytes for error logs. Should be a multiple of MAX_ERROR_LOG_STR_SIZE (see log.h)
#define HEARTBEAT_TLM_FS_ALLOCATION 10000 //1250000 // 10 MBit to bytes - should be a multiple of MAX_HB_TLM_FILE_SIZE (see hb_tlm_log.c)

extern TaskHandle_t xFilesystemTaskHandler;

/* Types and Globals */
typedef enum fs_operation_type {
    MAKE_FILESYSTEM,
    READ,
    WRITE,
    LIST_DIRECTORY,
    DELETE,
    MAKE_DIRECTORY,
    OPEN_DIRECTORY,
    READ_DIRECTORY,
    STAT
} fs_operation_type_t;

typedef struct {
    const char file_name[MAX_PATH_SIZE];
    char* read_buffer;
    size_t size;
    int32_t *out_size;
    TaskHandle_t calling_task;
} read_operation_t;

typedef struct {
    const char file_name[MAX_PATH_SIZE];
    char data[MAX_WRITE_CONTENTS_SIZE];
    size_t size;
    bool append_flag;
} write_operation_t;

typedef struct {
    const char directory_name[MAX_PATH_SIZE];
} list_directory_operation_t;

typedef struct {
    const char file_name[MAX_PATH_SIZE];
} delete_operation_t;

typedef struct {
    const char directory_name[MAX_PATH_SIZE];
} make_directory_operation_t;

typedef struct {
    const char directory_name[MAX_PATH_SIZE];
    DIR *dir_ptr_out;
    int32_t *retval_out;
    TaskHandle_t calling_task;
} open_directory_operation_t;

typedef struct {
    DIR *dir_ptr;
    FILINFO *file_info_out;
    int32_t *retval_out;
    TaskHandle_t calling_task;
} read_directory_operation_t;

typedef struct {
    const char file_name[MAX_PATH_SIZE];
    FILINFO *file_info_out;
    int32_t *retval_out;
    TaskHandle_t calling_task;
} stat_operation_t;

typedef union {
    read_operation_t read_op;
    write_operation_t write_op;
    list_directory_operation_t ls_op;
    delete_operation_t delete_op;
    make_directory_operation_t mkdir_op;
    open_directory_operation_t open_dir_op;
    read_directory_operation_t read_dir_op;
    stat_operation_t stat_op;
} file_operation_union_t;

typedef struct {
    fs_operation_type_t operation_type;
    file_operation_union_t file_operation;
} filesystem_queue_operations_t;

typedef enum {
    MOUNT_ON_FIRST_ACCESS,
    FORCE_MOUNT
} mounting_options_t;

QueueHandle_t filesystem_queue;

/* User Functions */
void make_filesystem();
int32_t read_file(const char *file_name, char *result_buffer, size_t size);
void write_file(const char *file_name, char *data, size_t size, bool append_flag);
void list_dir(const char *directory_name);
void delete_file(const char *file_name);
void make_dir(const char *directory_name);
void touch(const char *file_name);
void cat(const char *file_name);
int32_t open_dir(const char *dir_name, DIR *dir_ptr_out);
int32_t read_dir(DIR *dir_ptr, FILINFO *file_info_out);
int32_t stat(const char* file_name, FILINFO *file_info_out);
bool dir_exists(const char* dir_name);
bool file_exists(const char* file_name);

/* Internal Functions */
void _mkfs();
int32_t _fread(const char *file_name, char *result_buffer, size_t size);
int32_t _fwrite(const char *file_name, const uint8_t *data, size_t size, bool append_flag);
void _fdelete(const char *file_name);
void _flist(const char *dir_name);
void _fmkdir(const char *dir_name);
int32_t _fdiropen(const char *dir_name, DIR *dir_ptr_out);
int32_t _fdirread(DIR *dir_ptr, FILINFO *file_info_out);
int32_t _fstat(const char *file_name, FILINFO *file_info_out);

void _test();

// Main Task
void filesystem_task(void* unused_arg);

// special file system logging
void fs_log(const char *str, ...); 