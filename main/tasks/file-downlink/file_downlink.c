#include "file_downlink.h"
#include "timing.h"
#include "telemetry.h"
#include "log.h"

#include <FreeRTOS.h>
#include <task.h>
#include <string.h>

void initialize_file_downlink(char *file_path) {
    file_downlink_queue_command_init_data_t command_data;
    strncpy(command_data.file_path, file_path, MAX_PATH_SIZE);

    file_downlink_queue_command_t command = {
        .queue_command_id = (uint8_t) CMD_ID_FILE_DOWNLINK_INIT,
    };
    memcpy(command.data, &command_data, sizeof(file_downlink_queue_command_init_data_t));

    if (file_downlink_queue) {
        xQueueSendToBack(file_downlink_queue, &command, portMAX_DELAY);
    }
}

void file_downlink_ack_command(char *file_path, uint16_t sequence_number) {
    file_downlink_queue_command_ack_data_t command_data = {
        .sequence_number = sequence_number
    };
    strncpy(command_data.file_path, file_path, MAX_PATH_SIZE);
    
    file_downlink_queue_command_t command = {
        .queue_command_id = (uint8_t) CMD_ID_FILE_DOWNLINK_INIT,
    };
    memcpy(command.data, &command_data, sizeof(file_downlink_queue_command_ack_data_t));

    if (file_downlink_queue) {
        xQueueSendToBack(file_downlink_queue, &command, portMAX_DELAY);
    }
}

void change_max_packet_size(uint8_t new_packet_size) {
    file_downlink_queue_command_change_packet_size_data_t command_data = {
        .new_packet_size = new_packet_size
    };
    file_downlink_queue_command_t command = {
        .queue_command_id = (uint8_t) CMD_ID_CHANGE_DOWNLINK_PACKET_SIZE,
    };
    memcpy(command.data, &command_data, sizeof(file_downlink_queue_command_change_packet_size_data_t));

    if (file_downlink_queue) {
        xQueueSendToBack(file_downlink_queue, &command, portMAX_DELAY);
    }
}

// Keeps track of the current file being downlinked and all of the data needed for the state machine
typedef struct current_file_downlink_data {
    uint16_t window_start;
    uint16_t window_end;
    char file_path[MAX_PATH_SIZE];
    //FILE *file;
    uint16_t current_sending_packet_index;
    bool reached_eof; // Used to determine if the packet of the file has been sent

    // 2 separate times are kept, last_ack_received_time for timing out the entire file transfer and expiration_timer for timing out the current window transmission
    unsigned long last_ack_received_time; // ticks in ms since last ack received
    unsigned long expiration_timer; // same as last_ack_received_time except it is also reset after this time expires
} current_file_downlink_data_t;

// Sends 1 chunk of a file in a packet depending on the current state of the file downlink
// Returns 0 on success, 1 on end of file and negative numbers for errors
int send_n_packet(current_file_downlink_data_t *current_file_data, uint32_t packet_size) {

    file_downlink_telemetry_t *downlink_packet = (file_downlink_telemetry_t*) pvPortMalloc(sizeof(file_downlink_telemetry_t) + packet_size);
    strncpy(downlink_packet->path_name, current_file_data->file_path, FILE_DOWNLINK_PATH_NAME_CHARS - 1);
    downlink_packet->path_name[FILE_DOWNLINK_PATH_NAME_CHARS - 1] = '\0'; // Make sure there is a null-terminator - this may be unnecessary

    uint32_t bytes_read = read_file_offset(current_file_data->file_path, downlink_packet->data, packet_size, current_file_data->current_sending_packet_index * packet_size);

    if (bytes_read < 0) {
        vPortFree(downlink_packet);
        return bytes_read; // Error
    }
    
    downlink_packet->sequence_number = current_file_data->current_sending_packet_index; // packet index is incremented in the state machine loop
    send_telemetry(FILE_DOWNLINK_APID, (char*) downlink_packet, sizeof(file_downlink_telemetry_t) + bytes_read);
    
    if (bytes_read < packet_size) {
        vPortFree(downlink_packet);
        return 1; // End of file
    } else {
        return 0; // Success
    }
}

// Helper function to setup new transfer
void initialize_file_transfer(current_file_downlink_data_t *current_file_data_struct, file_downlink_queue_command_t *queue_command) {
    unsigned long tick_time_ms = tick_uptime_in_ms();

    file_downlink_queue_command_init_data_t *init_command = (file_downlink_queue_command_init_data_t*) queue_command->data;

    strncpy(current_file_data_struct->file_path, init_command->file_path, MAX_PATH_SIZE - 1);
    current_file_data_struct->file_path[MAX_PATH_SIZE - 1] = '\0'; // Make sure there is a null-terminator

    current_file_data_struct->window_start = 0;
    current_file_data_struct->window_end = WINDOW_SIZE_N;
    current_file_data_struct->last_ack_received_time = tick_time_ms;
    current_file_data_struct->expiration_timer = tick_time_ms;
    current_file_data_struct->current_sending_packet_index = 0;
    current_file_data_struct->reached_eof = false;
}

typedef enum file_downlink_state {
    IDLE,       // Waiting for a file downlink command
    SENDING,    // Sending packets and receiving acks, will eventually send entire file or timeout
} file_downlink_state_t;

void file_downlink_task(void* unused_arg) {

    uint32_t packet_downlink_size = MAX_DOWNLINK_PACKET_SIZE; // This can be changed by command when in idle state

    file_downlink_state_t state = IDLE; // start in the idle state
    current_file_downlink_data_t *current_file_downlink_data = NULL; // initialize all values to 0

    file_downlink_queue = xQueueCreate(FILE_DOWNLINK_MAX_QUEUE_ITEMS, sizeof(file_downlink_queue_command_t));
    file_downlink_queue_command_t *queue_command;
    // Send cool banner
    print_banner();
    while (true) {

        switch (state) {
            
            case IDLE:
                queue_command = NULL;
                xQueueReceive(file_downlink_queue, &queue_command, 0); // 0 for non blocking
                if (queue_command->queue_command_id == CMD_ID_FILE_DOWNLINK_INIT) {
                    initialize_file_transfer(current_file_downlink_data, queue_command);
                    state = SENDING;
                } else if (queue_command->queue_command_id == CMD_ID_CHANGE_DOWNLINK_PACKET_SIZE) {
                    file_downlink_queue_command_change_packet_size_data_t *change_packet_size_command = (file_downlink_queue_command_change_packet_size_data_t*) queue_command->data;
                    if (change_packet_size_command->new_packet_size < MAX_DOWNLINK_PACKET_SIZE) {
                        packet_downlink_size = change_packet_size_command->new_packet_size;
                    } else {
                        logln_error("File Downlink Task received an invalid packet size command: %d", change_packet_size_command->new_packet_size);
                    }
                } else {
                    logln_error("File Downlink Task received an invalid command in IDLE state: %d", queue_command->queue_command_id);
                }
                break;

            
            case SENDING:
                // If we have not sent all packets in the window, send the next packet in the window
                if (current_file_downlink_data->current_sending_packet_index < current_file_downlink_data->window_end) {
                    int result = send_n_packet(current_file_downlink_data, packet_downlink_size);
                    switch (result) {
                        case 0: // success
                            break;
                        case 1: // end of file
                            current_file_downlink_data->reached_eof = true;
                            break;
                        default: // error
                            logln_error("%d - Error sending packet in file downlink task", result);
                            current_file_downlink_data = NULL; // reset struct
                            state = IDLE;
                            break;
                    }
                    current_file_downlink_data->current_sending_packet_index++;
                }

                // Check for acks if the timer has not expired
                if (ticks_to_ms(tick_uptime_in_ms() - current_file_downlink_data->expiration_timer) < ACK_WINDOW_TIMEOUT) {
                    queue_command = NULL;
                    xQueueReceive(file_downlink_queue, &queue_command, 0); // 0 for non blocking
                    if (queue_command->queue_command_id == CMD_ID_FILE_DOWNLINK_ACK) {

                        file_downlink_queue_command_ack_data_t *ack_command_data = (file_downlink_queue_command_ack_data_t*) queue_command->data;

                        // See if the seq number is in the window. If so, check if this is the last ack of the transfer, otherwise update the start of the window
                        if (ack_command_data->sequence_number >= current_file_downlink_data->window_start &&
                                ack_command_data->sequence_number <= current_file_downlink_data->window_end) {

                            // Check if it is the end of the transfer
                            if (current_file_downlink_data->reached_eof == true && ack_command_data->sequence_number == current_file_downlink_data->window_end) {
                                current_file_downlink_data = NULL; // reset struct
                                state = IDLE;
                                break;
                            }

                            current_file_downlink_data->window_start = ack_command_data->sequence_number + 1;
                            current_file_downlink_data->window_end = current_file_downlink_data->window_start + WINDOW_SIZE_N;
                            current_file_downlink_data->current_sending_packet_index = current_file_downlink_data->window_start;

                            current_file_downlink_data->expiration_timer = tick_uptime_in_ms();
                            current_file_downlink_data->last_ack_received_time = tick_uptime_in_ms();
                        }
                    // If we receive a new file downlink request, terminate this transfer and start the new one
                    } else if (queue_command->queue_command_id == CMD_ID_FILE_DOWNLINK_INIT) {
                        logln_error("Received a new file downlink command while in the middle of a transfer");
                        current_file_downlink_data = NULL; // reset struct
                        initialize_file_transfer(current_file_downlink_data, queue_command); // continue with new transfer
                        break;
                    }
                } else {
                    // Resend packets starting at the last in-order ack received sequence number
                    current_file_downlink_data->current_sending_packet_index = current_file_downlink_data->window_start;
                    current_file_downlink_data->expiration_timer = tick_uptime_in_ms(); // Only update the timer
                }

                // Check if the entire file transfer has timed out
                if (ticks_to_ms(tick_uptime_in_ms() - current_file_downlink_data->last_ack_received_time) < ACK_TRANSFER_TIMEOUT) {
                    current_file_downlink_data = NULL; // reset struct
                    state = IDLE;
                }

                break;
        }

    }
}