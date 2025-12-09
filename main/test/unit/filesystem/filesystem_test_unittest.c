#include "FreeRTOS.h"
#include <string.h>

#include "unity.h"

#include "filesystem.h"

// Used on all tests to begin the filesystem task and to start the test task defined by each test (function pointer task_func_ptr)
typedef void (*TaskFunction_t)(void *);
static void test_log_rtos_scheduler_begin(TaskFunction_t task_func_ptr) {

    TaskHandle_t xFilesystemHandler = NULL; // Used for deleting after the test

    BaseType_t filesystem_task_status = xTaskCreate(filesystem_task,
                                        "FILESYSTEM",
                                        1024,
                                        NULL,
                                        1,
                                        &xFilesystemHandler);

    BaseType_t testing_task_status = xTaskCreate(task_func_ptr,
                                        "TEST",
                                        1024,
                                        xFilesystemHandler, 
                                        1,
                                        NULL); 

    // Start Filesystem scheduler and test task
    vTaskStartScheduler();

}

// Required at the end of each test task to exit correctly for the next test
static void end_test_task(TaskHandle_t xFilesystemHandler) {
    vTaskEndScheduler();
    vTaskDelete(xFilesystemHandler); // Delete filesystem task
    vTaskDelete(NULL); // Delete this current task
}


/****** Beginning of tests *******/


/* 
*  Test partial read of a file - read a file from an offset and compare the result
*/
static void test_filesystem_read_with_offset_testtask( void *pvParameters ) {

    TaskHandle_t xFilesystemHandler = (TaskHandle_t)pvParameters;

    // Write to file
    char* data = "This is a test file for partial read";
    write_file("test.txt", data, strlen(data), false); // write without appending

    // Read from file
    char result_buffer[10];
    int32_t read_size = read_file_offset("test.txt", result_buffer, 10, 5); // read from offset 5 (start with "is...")
    TEST_ASSERT_EQUAL(10, read_size); // Check if read size is correct

    // Compare strings
    TEST_ASSERT_EQUAL_STRING("is a test ", result_buffer);

    // Required at the end of each test job
    end_test_task(xFilesystemHandler);

}
void test_filesystem_read_with_offset_job( void ) {
    TEST_MESSAGE("Starting test test_filesystem_read_with_offset_job");
    test_log_rtos_scheduler_begin(test_filesystem_read_with_offset_testtask);
}
