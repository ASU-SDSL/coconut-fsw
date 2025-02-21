#include "file_downlink.h"
#include "timing.h"

#include <FreeRTOS.h>
#include <task.h>

void initialize_file_downlink(char *file_path) {
    file_downlink_queue_command_init_data_t command_data = {
        .file_path = file_path
    };
    file_downlink_queue_command_t command = {
        .queue_command = (uint8_t) FILE_DOWNLINK_INIT,
        .data = command_data
    };

    if (file_downlink_queue) {
        xQueueSendToBack(file_downlink_queue, &command, portMAX_DELAY);
    }
}

void file_downlink_ack_command(char *file_path, uint16_t sequence_number) {
    file_downlink_queue_command_ack_data_t command_data = {
        .file_path = file_path,
        .sequence_number = sequence_number
    };
    file_downlink_queue_command_t command = {
        .queue_command = (uint8_t) FILE_DOWNLINK_INIT,
        .data = command_data
    };

    if (file_downlink_queue) {
        xQueueSendToBack(file_downlink_queue, &command, portMAX_DELAY);
    }
}

void change_max_packet_size(uint8_t new_packet_size) {
    file_downlink_queue_command_change_packet_size_data_t command_data = {
        .new_packet_size = new_packet_size
    };
    file_downlink_queue_command_t command = {
        .queue_command = (uint8_t) CHANGE_DOWNLINK_PACKET_SIZE,
        .data = command_data
    };

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

    // 2 separate times are kept, last_ack_received_time for timing out the entire file transfer and expiration_timer for timing out the current window transmission
    unsigned long last_ack_received_time; // ticks in ms since last ack received
    unsigned long expiration_timer; // same as last_ack_received_time except it is also reset after this time expires
} current_file_downlink_data_t;

// Sends 1 chunk of a file in a packet depending on the current state of the file downlink
// Returns 0 on success, 1 on end of file and negative numbers for errors
int send_n_packet(current_file_downlink_data_t current_file_data) {

    

}

// Helper function to setup new transfer
void initialize_file_transfer(current_file_downlink_data_t *current_file_data_struct, file_downlink_queue_command_t *queue_command) {
    unsigned long tick_time_ms = tick_uptime_in_ms();

    current_file_data_struct->file_path = queue_command->file_path;
    current_file_data_struct->window_start = 0;
    current_file_data_struct->window_end = WINDOW_SIZE_N;
    current_file_data_struct->last_ack_received_time = tick_time_ms;
    current_file_data_struct->expiration_timer = tick_time_ms;
    current_file_data_struct->current_sending_packet_index = 0;
}

typedef enum file_downlink_state {
    IDLE,       // Waiting for a file downlink command
    SENDING,    // Sending packets and receiving acks, will eventually send entire file or timeout
} file_downlink_state_t;

void file_downlink_task(void* unused_arg) {

    file_downlink_state_t state = IDLE; // start in the idle state
    current_file_downlink_data_t current_file_downlink_data = {0}; // initialize all values to 0

    file_downlink_queue = xQueueCreate(FILE_DOWNLINK_MAX_QUEUE_ITEMS, sizeof(file_downlink_queue_command_t));
    file_downlink_queue_command_t queue_command;
    // Send cool banner
    print_banner();
    while (true) {

        switch (state) {
            
            case IDLE:
                queue_command = NULL;
                xQueueReceive(file_downlink_queue, &queue_command, 0); // 0 for non blocking
                if (queue_command.queue_command == FILE_DOWNLINK_INIT) {
                    initialize_file_transfer(&current_file_downlink_data, &queue_command);
                    state = SENDING;
                } else {
                    logln_error("File Downlink Task received an invalid command in IDLE state");
                }
                break;

            
            case SENDING:
                // If we have not sent all packets in the window, send the next packet in the window
                if (current_file_downlink_data.current_sending_packet_index < current_file_downlink_data.window_end) {
                    int result = send_n_packet(current_file_downlink_data);
                    if (result != 0) { // Error or end of file
                        if (result != 1) { // Error
                            logln_error("%d - Error sending packet in file downlink task", result);
                        }
                        current_file_downlink_data = {0}; // reset struct
                        state = IDLE;
                        break;
                    }
                    current_file_downlink_data.current_sending_packet_index++;
                }

                // Check for acks if the timer has not expired
                if (ticks_to_ms(tick_uptime_in_ms() - current_file_downlink_data.expiration_timer) < ACK_WINDOW_TIMEOUT) {
                    queue_command = NULL;
                    xQueueReceive(file_downlink_queue, &queue_command, 0); // 0 for non blocking
                    if (ack_command.queue_command == FILE_DOWNLINK_ACK) {

                        // See if the seq number is in the window and update the start of the window if so
                        if (ack_command.sequence_number >= current_file_downlink_data.window_start &&
                                ack_command.sequence_number <= current_file_downlink_data.window_end) {
                            current_file_downlink_data.window_start = ack_command.sequence_number + 1;
                            current_file_downlink_data.window_end = current_file_downlink_data.window_start + WINDOW_SIZE_N;
                            current_file_downlink_data.current_sending_packet_index = current_file_downlink_data.window_start;

                            current_file_downlink_data.expiration_timer = tick_uptime_in_ms();
                            current_file_downlink_data.last_ack_received_time = tick_uptime_in_ms();
                        }
                    }
                } else {
                    // Resend packets starting at the last in-order ack received sequence number
                    current_file_downlink_data.current_sending_packet_index = current_file_downlink_data.window_start;
                    current_file_downlink_data.expiration_timer = tick_uptime_in_ms(); // Only update the timer
                }

                // Check if the entire file transfer has timed out
                if (ticks_to_ms(tick_uptime_in_ms() - current_file_downlink_data.last_ack_received_time) < ACK_TRANSFER_TIMEOUT) {
                    current_file_downlink_data = {0}; // reset struct
                    state = IDLE;
                }

                break;
        }

    }
}