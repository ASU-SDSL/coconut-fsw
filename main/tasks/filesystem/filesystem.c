#include "filesystem.h"

/* USER FUNCTIONS */
char* read_file(const char* file_name) {

}

//write file
void write_file(const char* file_name, const char* text_to_write) {
    xSemaphoreTake(sd_mutex, portMAX_DELAY);
    // the third arg is the append_flag, since we are not appending -> 0
    write(file_name, text_to_write, 0);
    xSemaphoreGive(sd_mutex);
}

//append file
void append_file(const char* file_name, const char* text_to_append) {
    xSemaphoreTake(sd_mutex, portMAX_DELAY);
    // since we are appending, set append flag to 1
    write(file_name, text_to_append, 1);
    xSemaphoreGive(sd_mutex);
}

//create file
void create_file(const char* new_file_name) {

}

//delete file
void delete_file(const char* file_name) {
    xSemaphoreTake(sd_mutex, portMAX_DELAY);
    // since we are appending, set append flag to 1
    delete(file_name);
    xSemaphoreGive(sd_mutex);
}

void sd_task(void* unused_arg) {
    // init sd card mutex
    sd_mutex = xSemaphoreCreateMutex();

    //have a queue and other threads will queue up on operations
    //this thread will go through the queue and execute operations, if there are any
    //keeps looping until new operations are in queue

    // init queue and other stuff

    // start inf loop
    // check queue for queued operation
    // if operation is in queue, execute it

    // if no operation in queue, continue (after delay?)
}