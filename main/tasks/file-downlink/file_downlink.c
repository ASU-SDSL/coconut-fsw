#include "file_downlink.h"
#include "timing.h"
#include "telemetry.h"
#include "log.h"
#include "file_downlink_codes.h"

#include <FreeRTOS.h>
#include <task.h>
#include <string.h>

/* This return needs uint8_t to be able to be returned in an ack */
uint8_t initialize_file_downlink(char *file_path, int file_path_len) {
    // Initialize file downlink only has the string as a parameter

    // This should be checked in the command task but verify here in case there are other users of this function
    if (file_path_len > MAX_PATH_SIZE) {
        return FILE_DOWNLINK_PATH_OVERFLOW;
    }

    if (!file_exists(file_path)) {
        logln_error("No file '%s' on file downlink initialize command", file_path);
        return FILE_DOWNLINK_FILE_DOES_NOT_EXIST;
    }

    file_downlink_queue_command_t command = {
        .queue_command_id = (uint8_t) CMD_ID_FILE_DOWNLINK_INIT,
    };
    memcpy(command.data, file_path, file_path_len);

    // Make sure it is null terminated - this will not out of range from the MAX_PATH_SIZE check above
    // might be covered by file_downlink_queue_command_t initializing to 0s
    command.data[file_path_len] = '\0';

    logln_info("Message len: %d, sizeof: %d", file_path_len, sizeof(file_downlink_queue_command_t));

    if (file_downlink_queue) {
        xQueueSendToBack(file_downlink_queue, &command, portMAX_DELAY);
    }

    return 0;
}

void file_downlink_ack_command(uint8_t transaction_id, uint16_t sequence_number) {
    file_downlink_queue_command_ack_data_t command_data = {
        .transaction_id = transaction_id,
        .sequence_number = sequence_number
    };
    
    file_downlink_queue_command_t command = {
        .queue_command_id = (uint8_t) CMD_ID_FILE_DOWNLINK_ACK,
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
    uint8_t transaction_id;
    //FILE *file;
    uint16_t current_sending_packet_index;
    bool reached_eof; // Used to determine if the packet of the file has been sent

    // 2 separate times are kept, last_ack_received_time for timing out the entire file transfer and expiration_timer for timing out the current window transmission
    unsigned long last_ack_received_time; // ticks in ms since last ack received
    unsigned long expiration_timer; // same as last_ack_received_time except it is also reset after this time expires
} current_file_downlink_data_t;

// Sends 1 chunk of a file in a packet depending on the current state of the file downlink
// Returns 0 on success, SEND_N_PACKET_EOF on end of file or read_file_offset error
int send_n_packet(current_file_downlink_data_t *current_file_data, uint32_t packet_size, uint8_t transaction_id) {

    file_downlink_telemetry_t downlink_packet;
    strncpy(downlink_packet.path_name, current_file_data->file_path, FILE_DOWNLINK_PATH_NAME_CHARS - 1);
    downlink_packet.path_name[FILE_DOWNLINK_PATH_NAME_CHARS - 1] = '\0'; // Make sure there is a null-terminator - this may be unnecessary

    int bytes_read = read_file_offset(current_file_data->file_path, downlink_packet.data, packet_size, current_file_data->current_sending_packet_index * packet_size);

    if (bytes_read <= 0) {
        return bytes_read; // Error
    }
    
    downlink_packet.transaction_id = transaction_id;
    downlink_packet.sequence_number = current_file_data->current_sending_packet_index; // packet index is incremented in the state machine loop
    // Make sure we don't send more data from the buffer than we need to
    size_t buffer_empty_space = sizeof(downlink_packet.data) - bytes_read; // + 1; // + 1 for some weird indexing

    int return_code = 0; // Success
    downlink_packet.eof = false;
    if (bytes_read < packet_size) { // End of file
        return_code = SEND_N_PACKET_EOF;
        downlink_packet.eof = true;
    }

    char buf[256];
    memcpy(buf, downlink_packet.data, bytes_read);
    buf[bytes_read] = '\0';
    logln_info("Sending data: %s (%d) (%d)", sizeof(file_downlink_telemetry_t) - buffer_empty_space);

    send_telemetry(FILE_DOWNLINK_APID, (char*) &downlink_packet, sizeof(file_downlink_telemetry_t) - buffer_empty_space);

    logln_info("Downlink file message: %s", downlink_packet.data);
    
    return return_code;
}

// Helper function to setup new transfer
void initialize_file_transfer(current_file_downlink_data_t *current_file_data_struct, char *file_path, uint8_t *transaction_id) {
    unsigned long tick_time_ms = tick_uptime_in_ms();

    int file_path_len = MAX_PATH_SIZE;
    int len = strlen(file_path);
    if (len < MAX_PATH_SIZE - 1) {
        file_path_len = len;
    } else {
        len = MAX_PATH_SIZE - 1;
    }

    strncpy(current_file_data_struct->file_path, file_path, len+1); // +1 to copy the null terminator
    current_file_data_struct->file_path[MAX_PATH_SIZE - 1] = '\0'; // Make sure there is a null-terminator
    current_file_data_struct->transaction_id = *transaction_id;

    current_file_data_struct->window_start = 0;
    current_file_data_struct->window_end = WINDOW_SIZE_N;
    current_file_data_struct->last_ack_received_time = tick_time_ms;
    current_file_data_struct->expiration_timer = tick_time_ms;
    current_file_data_struct->current_sending_packet_index = 0;
    current_file_data_struct->reached_eof = false;

    // Increment transaction id for next transaction
    *transaction_id++;
}

typedef enum file_downlink_state {
    IDLE,       // Waiting for a file downlink command
    SENDING,    // Sending packets and receiving acks, will eventually send entire file or timeout
} file_downlink_state_t;

void file_downlink_task(void* unused_arg) {

    // @todo remove testing - For testing, log an error so the error log file exists
    logln_error("Error test");

    uint32_t packet_downlink_size = MAX_DOWNLINK_PACKET_SIZE; // This can be changed by command when in idle state

    file_downlink_state_t state = IDLE; // start in the idle state
    current_file_downlink_data_t current_file_downlink_data;
    uint8_t current_transaction_id = 0; // will be incremented for each new initialize file downlink command

    file_downlink_queue = xQueueCreate(FILE_DOWNLINK_MAX_QUEUE_ITEMS, sizeof(file_downlink_queue_command_t));
    file_downlink_queue_command_t queue_command;

    while (true) {

        switch (state) {
            
            case IDLE: {
                int res = xQueueReceive(file_downlink_queue, &queue_command, 0); // 0 for non blocking
                if (res == pdFALSE) {
                    continue;
                }
                if (queue_command.queue_command_id == CMD_ID_FILE_DOWNLINK_INIT) {
                    initialize_file_transfer(&current_file_downlink_data, queue_command.data, &current_transaction_id); // data contains only the path for this command
                    state = SENDING;
                } else if (queue_command.queue_command_id == CMD_ID_CHANGE_DOWNLINK_PACKET_SIZE) {
                    file_downlink_queue_command_change_packet_size_data_t *change_packet_size_command = (file_downlink_queue_command_change_packet_size_data_t*) queue_command.data;
                    if (change_packet_size_command->new_packet_size < MAX_DOWNLINK_PACKET_SIZE) {
                        packet_downlink_size = change_packet_size_command->new_packet_size;
                    } else {
                        logln_error("File Downlink Task received an invalid packet size command: %d", change_packet_size_command->new_packet_size);
                    }
                } else {
                    logln_error("File Downlink Task received an invalid command in IDLE state: %d", queue_command.queue_command_id);
                }
                break;
            }
            
            case SENDING: {
                // If we have not sent all packets in the window, send the next packet in the window
                if (current_file_downlink_data.current_sending_packet_index < current_file_downlink_data.window_end) {
                    int result = send_n_packet(&current_file_downlink_data, packet_downlink_size, current_transaction_id);
                    switch (result) {
                        case 0: // success
                            break;
                        case SEND_N_PACKET_EOF:
                            current_file_downlink_data.reached_eof = true;
                            break;
                        default: // error
                            logln_error("%d - Error sending packet in file downlink task", result);
                            state = IDLE;
                            break;
                    }
                    current_file_downlink_data.current_sending_packet_index++;
                }

                // Check for acks if the timer has not expired
                if (ticks_to_ms(tick_uptime_in_ms() - current_file_downlink_data.expiration_timer) < ACK_WINDOW_TIMEOUT) {
                    int res = xQueueReceive(file_downlink_queue, &queue_command, 0); // 0 for non blocking
                    if (res == pdFALSE) {
                        continue;
                    }
                    if (queue_command.queue_command_id == CMD_ID_FILE_DOWNLINK_ACK) {

                        file_downlink_queue_command_ack_data_t *ack_command_data = (file_downlink_queue_command_ack_data_t*) queue_command.data;

                        // Make sure this is still for the same transaction
                        if (ack_command_data->transaction_id != current_file_downlink_data.transaction_id) {
                            continue;
                        }

                        // See if the seq number is in the window. If so, check if this is the last ack of the transfer, otherwise update the start of the window
                        if (ack_command_data->sequence_number >= current_file_downlink_data.window_start &&
                                ack_command_data->sequence_number <= current_file_downlink_data.window_end) {

                            // Check if it is the end of the transfer
                            if (current_file_downlink_data.reached_eof == true && ack_command_data->sequence_number == current_file_downlink_data.window_end) {
                                logln_info("Last ack received: Transaction finished for %x", current_transaction_id);
                                state = IDLE;
                                break;
                            }

                            current_file_downlink_data.window_start = ack_command_data->sequence_number + 1;
                            current_file_downlink_data.window_end = current_file_downlink_data.window_start + WINDOW_SIZE_N;
                            current_file_downlink_data.current_sending_packet_index = current_file_downlink_data.window_start;

                            current_file_downlink_data.expiration_timer = tick_uptime_in_ms();
                            current_file_downlink_data.last_ack_received_time = tick_uptime_in_ms();
                            logln_info("ACK RECEIVED");
                        }
                    // If we receive a new file downlink request, terminate this transfer and start the new one
                    } else if (queue_command.queue_command_id == CMD_ID_FILE_DOWNLINK_INIT) {
                        logln_error("Received a new file downlink command while in the middle of a transfer");
                        initialize_file_transfer(&current_file_downlink_data, queue_command.data, &current_transaction_id); // continue with new transfer - data contains path
                        break;
                    }
                } else { // Ack timeout
                    // Resend packets starting at the last in-order ack received sequence number
                    current_file_downlink_data.current_sending_packet_index = current_file_downlink_data.window_start;
                    current_file_downlink_data.expiration_timer = tick_uptime_in_ms(); // Only update the timer
                }

                // Check if the entire file transfer has timed out
                if (ticks_to_ms(tick_uptime_in_ms() - current_file_downlink_data.last_ack_received_time) < ACK_TRANSFER_TIMEOUT) {
                    state = IDLE;
                }

                break;

            }
        }

    }
}