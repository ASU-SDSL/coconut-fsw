#include "filesystem.h"

/* USER FUNCTIONS */
void make_filesystem() {
    filesystem_queue_operations_t new_make_filesystem_operation;
    new_make_filesystem_operation.operation_type = MAKE_FILESYSTEM;

    while(filesystem_queue == NULL) {
        vTaskDelay(NULL_QUEUE_WAIT_TIME / portTICK_PERIOD_MS);
    }
    xQueueSendToBack(filesystem_queue, &new_make_filesystem_operation, portMAX_DELAY);
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

    // TODO: Block here until file read is finished
}

void write_file(const char* file_name, char* text_to_write) {
    filesystem_queue_operations_t new_write_operation;
    new_write_operation.operation_type = WRITE;
    new_write_operation.file_name = file_name;
    new_write_operation.text_to_write = text_to_write;

    while(filesystem_queue == NULL) {
        vTaskDelay(NULL_QUEUE_WAIT_TIME / portTICK_PERIOD_MS);
    }
    xQueueSendToBack(filesystem_queue, &new_write_operation, portMAX_DELAY);
}


/* INTERNAL FUNCTIONS */
size_t _mkfs() {
    void* buf = pvPortMalloc(0x400);
    FRESULT fr = f_mkfs("", NULL, buf, 0x400);    
    vPortFree(buf);
    return fr;
}

size_t _write(const char* file_name, const uint8_t *data, bool append_flag, size_t size) {
    FRESULT fr;
    FIL fil;

    int file_open_flags = FA_WRITE | FA_CREATE_ALWAYS;
    if(append_flag) { file_open_flags = FA_OPEN_APPEND | FA_WRITE; }

    // Open file for writing 
    fr = f_open(&fil, file_name, file_open_flags);
    while (fr != FR_OK) {
        logln_info("ERROR: Could not open file before write: %s (%d)\n", file_name, fr);
    }

    // Write something to file
    size_t bytes_written;
    fr = f_write(&fil, data, size, &bytes_written);
    if (fr) {
        logln_error("Could not write to file: %s (%d)\n", file_name, fr);
        f_close(&fil);
        return 0;
    }

    // Close file
    fr = f_close(&fil);
    while (fr != FR_OK) {
        logln_info("Could not close file after write: %s (%d)\n", file_name, fr);
    }
    return bytes_written;
}

size_t _read(const char *file_name, char *result_buffer, size_t size) { 
    // this function takes in a buffer where the result will be placed
    // the caller of this function is responsible for allocating the space for this buffer
    FRESULT fr;
    FIL fil;

    // open file
    fr = f_open(&fil, file_name, FA_READ);
    if (fr != FR_OK) {
        logln_error("Could not open file before read: %s (%d)\n", file_name, fr);
        return 0;
    }    

    // read from file
    size_t bytes_read;
    fr = f_read(&fil, result_buffer, size, &bytes_read);
    if (fr != FR_OK) {
        logln_error("Could not read from file: %s (%d)\n", file_name, fr);
        return 0;
    }

    // close file
    fr = f_close(&fil);
    if (fr != FR_OK) {
        logln_error("Could not close file after read: %s (%d)\n", file_name, fr);
    }
    return bytes_read;
}

void _delete(const char *file_name) {
    FRESULT fr;

    fr = f_unlink(file_name);
    if (fr != FR_OK) {
        logln_error("Could not remove file: %s (%d)\n", file_name, fr);
    }
}

void _list(const char *dir_name) {
    DIR dir;
    FRESULT fr;
 
    fr = f_opendir(&dir, dir_name);
 
    if (fr != FR_OK) {
        logln_error("Failed to open directory %s during ls (%d)\n", dir_name, fr);
        return;
    }
 
    for (;;) {
        FILINFO fno;

        fr = f_readdir(&dir, &fno);    
        
        if (fr != FR_OK) {
            printf("Failed to read directory %s during ls (%d)\n", dir_name, fr);
            return;
        }
        
        if (fno.fname[0] == 0) {
            return;
        }

        logln("%s\n", dir);
        logln("\t%c%c%c%c\t%10d\t%s/%s",
            ((fno.fattrib & AM_DIR) ? 'D' : '-'),
            ((fno.fattrib & AM_RDO) ? 'R' : '-'),
            ((fno.fattrib & AM_SYS) ? 'S' : '-'),
            ((fno.fattrib & AM_HID) ? 'H' : '-'),
            (int)fno.fsize, dir_name, fno.fname);
    }
}

void _test() {
    // write file
    char *test_filepath = "/test.txt";
    char *test_filecontents = "test\n";
    _write(test_filepath, test_filecontents, 0, strlen(test_filecontents));

    // read file
    char outbuf[0x10];
    size_t bytes_read = _read(test_filepath, outbuf, sizeof(outbuf));
    logln_info("Read %d bytes from %s: %s\n", bytes_read, test_filepath, outbuf);

    // ls
    _list("/");

    // delete
    logln_info("Deleting file %s\n", test_filepath);
    _delete(test_filepath);

    // ls
    _list("/");
}

void filesystem_task(void* unused_arg) {
    //have a queue and other threads will queue up on operations
    //this thread will go through the queue and execute operations, if there are any
    //keeps looping until new operations are in queue

    for (int i = 0; i < 5; i++) {
        logln_info("Im boutta blow\n");
        vTaskDelay(1000 / portTICK_PERIOD_MS);
    }

    // make filesystem
    void* buf = pvPortMalloc(0x400);
    FRESULT fr = f_mkfs("", NULL, buf, 0x400);    
    vPortFree(buf);

    // while(true) {
        logln_info("FS create status: %d\n", fr);
    //     vTaskDelay(1000 / portTICK_PERIOD_MS);
    // }
    
    // mount disk
    logln_info("Mounting filesystem...\n");
    FATFS fs; 
    fr = f_mount(&fs, "0:", 1);
    while (fr != FR_OK) {
        logln_error("Could not mount filesystem (%d)", fr);
        vTaskDelay(1000 / portTICK_PERIOD_MS);
    }
    

    _test();

    // init queue and other stuff
    filesystem_queue = xQueueCreate(FILESYSTEM_QUEUE_LENGTH, sizeof(filesystem_queue_operations_t));
    if(filesystem_queue == NULL) {
        // TODO: Find a better solution to handling queue creation failure
        vTaskDelete(NULL);
    }

    // start inf loop
    filesystem_queue_operations_t received_operation;
    while(1) {
        // check queue for queued operation
        // wait forever until an operation is in queue
        xQueueReceive(filesystem_queue, &received_operation, EMPTY_QUEUE_WAIT_TIME);

        // if operation is in queue, execute it
        switch (received_operation.operation_type) {
            case MAKE_FILESYSTEM:
                make_filesystem();
                break;
            case READ:
                _read(received_operation.file_name, received_operation.read_buffer, received_operation.size);
                break;
            case WRITE:
                _write(received_operation.file_name, received_operation.text_to_write, 0, received_operation.size);
                break;
            default:
                logln_error("Unrecognized file operation: %d\n", received_operation.operation_type);
                break;
        }
    }
}