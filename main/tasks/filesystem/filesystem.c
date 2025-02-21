#include <string.h>

#include "log.h"
#include "filesystem.h"

FATFS fs;

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
// returns -1 on failure
int32_t read_file(const char* file_name, char* result_buffer, size_t size) {
    if ((strnlen(file_name, MAX_PATH_SIZE) + 1) > MAX_PATH_SIZE) return 0;
    filesystem_queue_operations_t new_file_operation;
    new_file_operation.operation_type = READ;
    strncpy(new_file_operation.file_operation.read_op.file_name, file_name, MAX_PATH_SIZE);

    size_t out_size = 0;

    new_file_operation.file_operation.read_op.read_buffer = result_buffer;
    new_file_operation.file_operation.read_op.size = size;
    new_file_operation.file_operation.read_op.out_size = &out_size;
    new_file_operation.file_operation.read_op.calling_task = xTaskGetCurrentTaskHandle();

    while(filesystem_queue == NULL) {
        vTaskDelay(NULL_QUEUE_WAIT_TIME / portTICK_PERIOD_MS);
    }
    xQueueSendToBack(filesystem_queue, &new_file_operation, portMAX_DELAY);

    // Wait for notification that file read is finished
    uint32_t notification_retval = ulTaskNotifyTake(pdTRUE, NOTIFICATION_WAIT_TIME);
    if (notification_retval != 1) {
        logln_info("Timed out waiting for file read on %s task", get_current_task_name());
        return -1;
    }

    // Return size
    return (int32_t)out_size;
}

void write_file(const char* file_name, char* data, size_t size, bool append_flag) {
    if ((strnlen(file_name, MAX_PATH_SIZE) + 1) > MAX_PATH_SIZE) return;
    if (size > MAX_WRITE_CONTENTS_SIZE) return;
    filesystem_queue_operations_t new_file_operation;
    new_file_operation.operation_type = WRITE;
    strncpy(new_file_operation.file_operation.write_op.file_name, file_name, MAX_PATH_SIZE);
    memcpy(new_file_operation.file_operation.write_op.data, data, size);
    new_file_operation.file_operation.write_op.size = size;
    new_file_operation.file_operation.write_op.append_flag = append_flag;

    while(filesystem_queue == NULL) {
        vTaskDelay(NULL_QUEUE_WAIT_TIME / portTICK_PERIOD_MS);
    }
    xQueueSendToBack(filesystem_queue, &new_file_operation, portMAX_DELAY);
}

// sends a list of all files or subdirectories inside a directory to log_ln
void list_dir(const char* directory_name) {
    if ((strnlen(directory_name, MAX_PATH_SIZE) + 1) > MAX_PATH_SIZE) return;
    filesystem_queue_operations_t new_file_operation;
    new_file_operation.operation_type = LIST_DIRECTORY;
    strncpy(new_file_operation.file_operation.ls_op.directory_name, directory_name, MAX_PATH_SIZE);
    
    while(filesystem_queue == NULL) {
        vTaskDelay(NULL_QUEUE_WAIT_TIME / portTICK_PERIOD_MS);
    }
    xQueueSendToBack(filesystem_queue, &new_file_operation, portMAX_DELAY);
}

// deletes a file or directory
// directory must be empty to delete
void delete_file(const char* file_name) {
    if ((strnlen(file_name, MAX_PATH_SIZE) + 1) > MAX_PATH_SIZE) return;
    filesystem_queue_operations_t new_file_operation;
    new_file_operation.operation_type = DELETE;
    strncpy(new_file_operation.file_operation.delete_op.file_name, file_name, MAX_PATH_SIZE);

    while(filesystem_queue == NULL) {
        vTaskDelay(NULL_QUEUE_WAIT_TIME / portTICK_PERIOD_MS);
    }
    xQueueSendToBack(filesystem_queue, &new_file_operation, portMAX_DELAY);
}

// makes a directory
void make_dir(const char* directory_name) {
    if ((strnlen(directory_name, MAX_PATH_SIZE) + 1) > MAX_PATH_SIZE) return;
    filesystem_queue_operations_t new_file_operation;
    new_file_operation.operation_type = MAKE_DIRECTORY;
    strncpy(new_file_operation.file_operation.mkdir_op.directory_name, directory_name, MAX_PATH_SIZE);

    while(filesystem_queue == NULL) {
        vTaskDelay(NULL_QUEUE_WAIT_TIME / portTICK_PERIOD_MS);
    }
    xQueueSendToBack(filesystem_queue, &new_file_operation, portMAX_DELAY);
}

// creates an empty file
void touch(const char* file_name) {
    write_file(file_name, "", 0, 0);
}

// prints file contents to logln
void cat(const char* file_name) {
    char* result_buffer = pvPortMalloc(CAT_SIZE_LIMIT);
    size_t read_size = read_file(file_name, result_buffer, CAT_SIZE_LIMIT);
    if (read_size <= 0) return;
    _write_log(result_buffer, read_size);
    vPortFree(result_buffer);
}

// opens a directory to use in read_dir
// returns 0 on success, 1 on failure
int32_t open_dir(const char *dir_name, DIR *dir_ptr_out) {
    if (strnlen(dir_name, MAX_PATH_SIZE)+1 > MAX_PATH_SIZE) return 0;
    filesystem_queue_operations_t new_file_operation;
    new_file_operation.operation_type = OPEN_DIRECTORY;
    strncpy(new_file_operation.file_operation.open_dir_op.directory_name, dir_name, MAX_PATH_SIZE);

    int32_t retval = 0;

    new_file_operation.file_operation.open_dir_op.dir_ptr_out = dir_ptr_out;
    new_file_operation.file_operation.open_dir_op.retval_out = &retval;
    new_file_operation.file_operation.open_dir_op.calling_task = xTaskGetCurrentTaskHandle();

    while(filesystem_queue == NULL) {
        vTaskDelay(NULL_QUEUE_WAIT_TIME / portTICK_PERIOD_MS);
    }
    xQueueSendToBack(filesystem_queue, &new_file_operation, portMAX_DELAY);

    // Wait for notification that file read is finished
    uint32_t notification_retval = ulTaskNotifyTake(pdTRUE, NOTIFICATION_WAIT_TIME);
    if (notification_retval != 1) {
        logln_info("Timed out waiting for directory open on %s task", get_current_task_name());
        return 1;
    }

    // Return size
    return retval;
}

// reads a dir that was opened in open_dir
// meant to be called multiple times on the same DIR struct to read all files inside a directory
// returns 0 on success, 1 on failure
// returns FatFS struct; find usage examples of f_readdir online for more info 
int32_t read_dir(DIR *dir_ptr, FILINFO *file_info_out) {
    filesystem_queue_operations_t new_file_operation;
    new_file_operation.operation_type = READ_DIRECTORY;

    int32_t retval = 0;

    new_file_operation.file_operation.read_dir_op.dir_ptr = dir_ptr;
    new_file_operation.file_operation.read_dir_op.file_info_out = file_info_out;
    new_file_operation.file_operation.read_dir_op.retval_out = &retval;
    new_file_operation.file_operation.read_dir_op.calling_task = xTaskGetCurrentTaskHandle();

    while(filesystem_queue == NULL) {
        vTaskDelay(NULL_QUEUE_WAIT_TIME / portTICK_PERIOD_MS);
    }
    xQueueSendToBack(filesystem_queue, &new_file_operation, portMAX_DELAY);

    // Wait for notification that file read is finished
    uint32_t notification_retval = ulTaskNotifyTake(pdTRUE, NOTIFICATION_WAIT_TIME);
    if (notification_retval != 1) {
        logln_info("Timed out waiting for directory read on %s task", get_current_task_name());
        return 1;
    }

    // Return size
    return retval;
}

// gets stats on a file
// returns 0 on success, 1 on failure
// returns FatFS struct; find usage examples of f_stat online for more info 
int32_t stat(const char* file_name, FILINFO *file_info_out) {
    if (strnlen(file_name, MAX_PATH_SIZE)+1 > MAX_PATH_SIZE) return 0;
    filesystem_queue_operations_t new_file_operation;
    new_file_operation.operation_type = STAT;
    strncpy(new_file_operation.file_operation.stat_op.file_name, file_name, MAX_PATH_SIZE);

    int32_t retval = 0;

    new_file_operation.file_operation.stat_op.file_info_out = file_info_out;
    new_file_operation.file_operation.stat_op.retval_out = &retval;
    new_file_operation.file_operation.stat_op.calling_task = xTaskGetCurrentTaskHandle();

    while(filesystem_queue == NULL) {
        vTaskDelay(NULL_QUEUE_WAIT_TIME / portTICK_PERIOD_MS);
    }
    xQueueSendToBack(filesystem_queue, &new_file_operation, portMAX_DELAY);

    // Wait for notification that file read is finished
    uint32_t notification_retval = ulTaskNotifyTake(pdTRUE, NOTIFICATION_WAIT_TIME);
    if (notification_retval != 1) {
        logln_info("Timed out waiting for file stat on %s task", get_current_task_name());
        return 1;
    }

    // Return size
    return retval;
}

// checks if a directory exists
// if the provided path does not exist or is a file, return false
bool dir_exists(const char* dir_name) {
    FILINFO file_info;
    if (stat(dir_name, &file_info)) {
        return false;
    }
    return !!(file_info.fattrib & AM_DIR);
}

// checks if a file exists
// if the provided path does not exist or is a directory, return false
bool file_exists(const char* file_name) {
    FILINFO file_info;
    if (stat(file_name, &file_info)) {
        return false;
    }
    return !(file_info.fattrib & AM_DIR);
}

/* INTERNAL FUNCTIONS */
void _mkfs() {
    FRESULT fr;

    logln_info("Making filesystem...\n");
    void* buf = pvPortMalloc(0x400);
    fr = f_mkfs("", NULL, buf, 0x400);
    vPortFree(buf);
    if (fr != FR_OK) {
        logln_error("Failed to make filesystem (%d)", fr);
        return;
    }
    
    // mount disk
    logln_info("Mounting filesystem...\n");
    fr = f_mount(&fs, "0:", 1);
    if (fr != FR_OK) {
        logln_error("Failed to mount filesystem (%d)", fr);
        return;
    }
}

int32_t _fwrite(const char* file_name, const uint8_t *data, size_t size, bool append_flag) {
    FRESULT fr;
    FIL fil;

    logln_info("Writing to file: %s\n", file_name);

    int file_open_flags = FA_WRITE | FA_CREATE_ALWAYS;
    if (append_flag) { file_open_flags = FA_OPEN_APPEND | FA_WRITE; }

    // Open file for writing 
    fr = f_open(&fil, file_name, file_open_flags);
    if (fr != FR_OK) {
        logln_info("ERROR: Could not open file before write: %s (%d)\n", file_name, fr);
        return -1;
    }

    // Write something to file
    int32_t bytes_written = -1;
    fr = f_write(&fil, data, size, &bytes_written);
    if (fr != FR_OK) {
        logln_error("Could not write to file: %s (%d)\n", file_name, fr);
        bytes_written = -1;
    }

    // Close file
    fr = f_close(&fil);
    if (fr != FR_OK) {
        logln_info("Could not close file after write: %s (%d)\n", file_name, fr);
        bytes_written = -1;
    }
    return bytes_written;
}

int32_t _fread(const char *file_name, char *result_buffer, size_t size) { 
    // this function takes in a buffer where the result will be placed
    // the caller of this function is responsible for allocating the space for this buffer
    FRESULT fr;
    FIL fil;

    // open file
    fr = f_open(&fil, file_name, FA_READ);
    if (fr != FR_OK) {
        logln_error("Could not open file before read: %s (%d)\n", file_name, fr);
        return -1;
    }    

    // read from file
    int32_t bytes_read = -1;
    fr = f_read(&fil, result_buffer, size, &bytes_read);
    if (fr != FR_OK) {
        logln_error("Could not read from file: %s (%d)\n", file_name, fr);
        bytes_read = -1;
    }

    // close file
    fr = f_close(&fil);
    if (fr != FR_OK) {
        logln_error("Could not close file after read: %s (%d)\n", file_name, fr);
        bytes_read = -1;
    }
    return bytes_read;
}

// Reads a file from an offset
int32_t _fread_offset(const char *file_name, char *result_buffer, size_t size, uint32_t offset) { 
    // this function takes in a buffer where the result will be placed
    // the caller of this function is responsible for allocating the space for this buffer
    FRESULT fr;
    FIL fil;

    // open file
    fr = f_open(&fil, file_name, FA_READ);
    if (fr != FR_OK) {
        logln_error("Could not open file before read: %s (%d)\n", file_name, fr);
        return -1;
    }

    // Try to incremenet the file pointer to the offset
    fr = f_lseek(&fil, offset);
    if (fr != FR_OK) {
        logln_error("Could not seek to offset: %s (%d)\n", file_name, fr);
        return -1;
    }

    // read from file
    int32_t bytes_read = -1;
    fr = f_read(&fil, result_buffer, size, &bytes_read);
    if (fr != FR_OK) {
        logln_error("Could not read from file: %s (%d)\n", file_name, fr);
        bytes_read = -1;
    }

    // close file
    fr = f_close(&fil);
    if (fr != FR_OK) {
        logln_error("Could not close file after read: %s (%d)\n", file_name, fr);
        bytes_read = -1;
    }
    return bytes_read;
}

void _fdelete(const char *file_name) {
    FRESULT fr;

    fr = f_unlink(file_name);
    if (fr != FR_OK) {
        logln_error("Could not remove file: %s (%d)\n", file_name, fr);
    }
}

void _flist(const char *dir_name) {
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
            logln_error("Failed to read directory %s during ls (%d)\n", dir_name, fr);
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

void _fmkdir(const char *dir_name) {
    FRESULT fr;

    fr = f_mkdir(dir_name);
    if (fr != FR_OK) {
        logln_error("Failed to make directory %s during mkdir (%d)\n", dir_name, fr);
    }
}

int32_t _fdiropen(const char *dir_name, DIR *dir_ptr_out) {
    FRESULT fr = f_opendir(dir_ptr_out, dir_name);
 
    if (fr != FR_OK) {
        logln_error("Failed to open directory %s during fdiropen (%d)\n", dir_name, fr);
        return 1;
    }
    return 0;
}

int32_t _fdirread(DIR *dir_ptr, FILINFO *file_info_out) {
    FRESULT fr = f_readdir(dir_ptr, file_info_out);    
    
    if (fr != FR_OK) {
        logln_error("Failed to read directory during fdirread (%d)\n", fr);
        return 1;
    }
    return 0;
}

int32_t _fstat(const char *file_name, FILINFO *file_info_out) {
    FRESULT fr = f_stat(file_name, file_info_out);    
    
    if (fr != FR_OK) {
        // logln_error("Failed to stat %s during fstat (%d)\n", file_name, fr);
        return 1;
    }
    return 0;
}

void _test() {
    // write file
    char *test_filepath = "/test.txt";
    char *test_filecontents = "test\n";
    _fwrite(test_filepath, test_filecontents, 0, strlen(test_filecontents));

    // read file
    char outbuf[0x10];
    size_t bytes_read = _fread(test_filepath, outbuf, sizeof(outbuf));
    logln_info("Read %d bytes from %s: %s\n", bytes_read, test_filepath, outbuf);

    // ls
    _flist("/");

    // delete
    logln_info("Deleting file %s\n", test_filepath);
    _fdelete(test_filepath);

    // ls
    _flist("/");
}

void filesystem_task(void* unused_arg) {
    
#ifdef SIMULATOR
    _mkfs();
#endif
    
    // mount disk
    FATFS fs;
    FRESULT fr = f_mount(&fs, "0:", 1);
    if (fr != FR_OK) {
        logln_warn("Failed to mount filesystem, you probably need to run make_filesystem (mkfs)!", fr);
    }

    // init queue
    filesystem_queue = xQueueCreate(FILESYSTEM_QUEUE_LENGTH, sizeof(filesystem_queue_operations_t));
    if(filesystem_queue == NULL) {
        // TODO: Find a better solution to handling queue creation failure
        vTaskDelete(NULL);
    }

    // start inf loop
    filesystem_queue_operations_t received_operation;
    while(1) {
        // wait until an operation is in queue
        xQueueReceive(filesystem_queue, &received_operation, EMPTY_QUEUE_WAIT_TIME);
        // parse and execute operation
        switch (received_operation.operation_type) {
            case MAKE_FILESYSTEM: {
                _mkfs();
                break;
            }
            case READ: {
                read_operation_t read_op = received_operation.file_operation.read_op;
                size_t read_size = _fread(read_op.file_name, read_op.read_buffer, read_op.size);
                *read_op.out_size = read_size;
                xTaskNotifyGive(read_op.calling_task);
                break;
            }
            case WRITE: {
                write_operation_t write_op = received_operation.file_operation.write_op;
                _fwrite(write_op.file_name, write_op.data, write_op.size, write_op.append_flag);
                break;
            }
            case LIST_DIRECTORY: {
                list_directory_operation_t ls_op = received_operation.file_operation.ls_op;
                _flist(ls_op.directory_name);
                break;
            }
            case DELETE: {
                delete_operation_t delete_op = received_operation.file_operation.delete_op;
                _fdelete(delete_op.file_name);
                break;
            }
            case MAKE_DIRECTORY: {
                make_directory_operation_t mkdir_op = received_operation.file_operation.mkdir_op;
                _fmkdir(mkdir_op.directory_name);
                break;
            }
            case OPEN_DIRECTORY: {
                open_directory_operation_t open_dir_op = received_operation.file_operation.open_dir_op;
                int32_t retval = _fdiropen(open_dir_op.directory_name, open_dir_op.dir_ptr_out); 
                *open_dir_op.retval_out = retval;
                xTaskNotifyGive(open_dir_op.calling_task);
                break;
            }
            case READ_DIRECTORY: {
                read_directory_operation_t read_dir_op = received_operation.file_operation.read_dir_op;
                int32_t retval = _fdirread(read_dir_op.dir_ptr, read_dir_op.file_info_out);
                *read_dir_op.retval_out = retval;
                xTaskNotifyGive(read_dir_op.calling_task);
                break;
            }
            case STAT: {
                stat_operation_t stat_op = received_operation.file_operation.stat_op;
                int32_t retval = _fstat(stat_op.file_name, stat_op.file_info_out);
                *stat_op.retval_out = retval;
                xTaskNotifyGive(stat_op.calling_task);
                break;
            }
            default: {
                logln_error("Unrecognized file operation: %d\n", received_operation.operation_type);
                break;
            }
        }
    }
}
