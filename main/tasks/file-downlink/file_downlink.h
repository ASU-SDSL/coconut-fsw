#include "FreeRTOS.h"
#include "queue.h"

#include "filesystem.h"

/*
*  File Downlink Task
*   - This task is responsible for sending files to the ground on command
*   - This task uses a simple custom protocol based on go-back-n
*
*  The protocol:
*   - The ground sends a "file downlink initialization" command
*   - The satellite responds with the first n packets of the file
*   - The ground sends an ack command after every n packet is received, specified by the sequence number (for the file, unrelated to ccsds space packet)
*   - The satellite will increment the window of packets it sends when acks are received in order
*   - If the satellite receives all acks up to the most recently sent sequence number within ACK_TIMEOUT, it immediately sends the next n packets
*   - If the satellite does not receive all acks within ACK_TIMEOUT, it will send the packets starting at the last in-order ack received sequence number
*/

#define FILE_DOWNLINK_MAX_QUEUE_ITEMS 128
#define FILE_DOWNLINK_CHECK_DELAY_MS portMAX_DELAY

#define WINDOW_SIZE_N 5 // size of the window in the go-back-n protocol - should probably be configurable in flight
#define ACK_WINDOW_TIMEOUT 5000 // ms to wait for an ack before resending the window
#define ACK_TRANSFER_TIMEOUT 600000 // ms to wait since the last received ack to cancel the transfer

#define MAX_DOWNLINK_PACKET_SIZE 256 // size of each packet in the file downlink protocol until the last packet - this can be commanded to be smaller

QueueHandle_t file_downlink_queue;

enum file_downlink_queue_command_id {
    FILE_DOWNLINK_INIT = 0,
    FILE_DOWNLINK_ACK = 1,
    CHANGE_DOWNLINK_PACKET_SIZE = 2,
};

typedef struct file_downlink_queue_command_init_data {
    char file_path[MAX_PATH_SIZE];
} file_downlink_queue_command_init_data_t;

typedef struct file_downlink_queue_command_ack_data {
    uint16_t sequence_number;
    char file_path[MAX_PATH_SIZE];
} file_downlink_queue_command_ack_data_t;

typedef struct file_downlink_queue_command_change_packet_size_data {
    uint8_t new_packet_size;
} file_downlink_queue_command_change_packet_size_data_t;

typedef struct file_downlink_queue_command {
    uint8_t queue_command_id;
    uint8_t data[sizeof(file_downlink_queue_command_ack_data_t)]; // Dynamic data with max size of the init data with the max file path
} file_downlink_queue_command_t;


void file_downlink_task();

/* API Functions */

/*
*  Used by the command task to initialize a file downlink - starts transmitting with sequence_number 0
*/
void initialize_file_downlink(char *file_path);

/*
*  Used by the command task when an ack is received from the ground after n packets are sent
*/
void file_downlink_ack_command(char *file_path, uint16_t sequence_number);

/*
*  Used by the command task to change the max packet size (for a single packet n) of the file downlink protocol
*/
void change_max_packet_size(uint8_t new_packet_size);
