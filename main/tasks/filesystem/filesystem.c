#include "filesystem.h"

/* USER FUNCTIONS */

// write file
void write_file(const char* file_name, char* text_to_write) {
    // create new blank write operation and fill it with correct values
    filesystem_queue_operations_t new_write_operation;
    new_write_operation.operation_type = WRITE;
    // might need to dynamically allocate file_name and text_to_write
    // might go out of scope in caller and be deallocated while our function still needs it
    new_write_operation.file_name = file_name;
    new_write_operation.text_to_write = text_to_write;

    // wait for queue to be initialized
    while(filesystem_queue == NULL) {
        vTaskDelay(NULL_QUEUE_WAIT_TIME / portTICK_PERIOD_MS);
    }
    // after queue is initialized, add write op onto queue
    xQueueSendToBack(filesystem_queue, &new_write_operation, portMAX_DELAY);
}

// this function takes in a buffer as a parameter to store the result from the read
// the caller is responsible for allocating memory for this buffer and freeing the memory
void read_file(const char* file_name, char* result_buffer) {
    filesystem_queue_operations_t new_read_operation;
    new_read_operation.operation_type = READ;

    new_read_operation.file_name = file_name;
    new_read_operation.read_buffer = result_buffer;

    while(filesystem_queue == NULL) {
        vTaskDelay(NULL_QUEUE_WAIT_TIME / portTICK_PERIOD_MS);
    }
    xQueueSendToBack(filesystem_queue, &new_read_operation, portMAX_DELAY);
}

// //append file
// void append_file(const char* file_name, const char* text_to_append) {
//     xSemaphoreTake(sd_mutex, portMAX_DELAY);
//     // since we are appending, set append flag to 1
//     write(file_name, text_to_append, 1);
//     xSemaphoreGive(sd_mutex);
// }

// //create file
// void create_file(const char* new_file_name) {

// }

// //delete file
// void delete_file(const char* file_name) {
//     xSemaphoreTake(sd_mutex, portMAX_DELAY);
//     // since we are appending, set append flag to 1
//     delete(file_name);
//     xSemaphoreGive(sd_mutex);
// }

void sd_task(void* unused_arg) {
    //have a queue and other threads will queue up on operations
    //this thread will go through the queue and execute operations, if there are any
    //keeps looping until new operations are in queue

    // init queue and other stuff
    filesystem_queue = xQueueCreate(FILESYSTEM_QUEUE_LENGTH, sizeof(filesystem_queue_operations_t));
    if(filesystem_queue == NULL) {
        // TODO: Find a better solution to handling queue creation failure
        vTaskDelete(NULL);
    }
    logln_info("Queue created\r\n");

    // start inf loop
    filesystem_queue_operations_t received_operation;
    while(1) {
        // check queue for queued operation
        // wait forever until an operation is in queue
        xQueueReceive(filesystem_queue, &received_operation, EMPTY_QUEUE_WAIT_TIME);

        // if operation is in queue, execute it
        switch (received_operation.operation_type) {
            case WRITE:
                // execute write operation
                sd_write(received_operation.file_name, received_operation.text_to_write, 0);
                break;
            case READ:
                // execute read operation
                sd_read(received_operation.file_name, received_operation.read_buffer);
                break;
            default:
                // TODO: figure out proper way to handle this error
                break;
        }

        

    }
}