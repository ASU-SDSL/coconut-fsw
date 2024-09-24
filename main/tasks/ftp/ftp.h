#pragma once

#include "pico/stdlib.h"

#include <stdio.h>
#include <string.h>

#include <FreeRTOS.h>
#include <task.h>
#include <semphr.h>
#include "queue.h"
#include "gse.h"
#include "ccsds.h"
#include "log.h"
#include "steve.h"

#define FTP_MAX_QUEUE_ITEMS 64

// CCSDS payload FTP struct (bytes)
// 0x0: FTP operation
// 0x1 - 0x2: User creds length
// 0x3 - 0xX: User creds
// 0xX+1 - 0xX+2: Path length << only on initial packet
// 0xX+3 - 0xY: Path << only on initial packet
// Multi-byte numbers stored in big-endian

// FTP Structs and Types

// Potentially should be moved to FS header
typedef enum ftp_perms {
    PERM_CREATE = 1 << 0,
    PERM_READ = 1 << 1,
    PERM_UPDATE = 1 << 2,
    PERM_DELETE = 1 << 3,
} ftp_perms_t;

typedef enum ftp_operation {
    FTP_CREATE = 0, // Create file, error if exists
    FTP_APPEND = 1, // Append to existing file, error if does not exist
    FTP_COPY = 2, // Transmit FSW to GSE
    FTP_LIST = 3, // List files to GSE
    FTP_REMOVE = 4, // Delete file
} ftp_operation_t;

// Minimal struct for queue
typedef struct {
    uint32_t size;
    char* buffer;
} ftp_queue_operations_t;

// Full state description
typedef struct {
    uint32_t size; // Total size of buffer
    uint16_t path_length; // Length of file path
    uint16_t creds_length; // Length of user credentials

    ftp_operation_t op; // Transaction operation

    char* path; // File path
    char* creds; // User credentials

    char* buffer; // For cleanup
} ftp_state;

QueueHandle_t ftp_queue;

/* USER FUNCTIONS */
void ftp_queue_message(char *buffer, size_t size);

// Main Task
void ftp_task(void* unused_arg);
