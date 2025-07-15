#ifndef FILE_DOWNLINK_H
#define FILE_DOWNLINK_H

#include "FreeRTOS.h"
#include "filesystem.h"
#include "queue.h"
#include "timing.h"

#define FILE_DOWNLINK_MAX_QUEUE_ITEMS 128
#define FILE_DOWNLINK_CHECK_DELAY_MS portMAX_DELAY

#define WINDOW_SIZE_N 5
#define ACK_WINDOW_TIMEOUT 5000
#define ACK_TRANSFER_TIMEOUT 600000

#define MAX_DOWNLINK_PACKET_SIZE 256

// global queue?
extern QueueHandle_t file_downlink_queue;

typedef enum {
  FILE_DOWNLINK_INIT = 0,
  FILE_DOWNLINK_ACK = 1,
  CHANGE_DOWNLINK_PACKET_SIZE = 2,
} file_downlink_queue_command;

typedef struct {
  char file_path[MAX_PATH_SIZE];
} file_downlink_queue_command_init_data_t;

typedef struct {
  uint16_t sequence_number;
  char file_path[MAX_PATH_SIZE];
} file_downlink_queue_command_ack_data_t;

typedef struct {
  uint8_t new_packet_size;
} file_downlink_queue_command_change_packet_size_data_t;

typedef struct {
  uint8_t queue_command;
  uint8_t data[sizeof(file_downlink_queue_command_ack_data_t)];
} file_downlink_queue_command_t;

TickType_t tick_uptime_in_ms();

// void file_downlink_task(void);
void file_downlink_task(void *unused);
void initialize_file_downlink(char *file_path);
void file_downlink_ack_command(char *file_path, uint16_t sequence_number);
void change_max_packet_size(uint8_t new_packet_size);

#endif
