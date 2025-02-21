#include "FreeRTOS.h"
#include <string.h>

#include "unity.h"

#include "filesystem.h"
#include "log.h"

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
*  Log a single error and check if it is retrieved correctly
*/
static void test_log_single_error_testtask( void *pvParameters ) {

    TaskHandle_t xFilesystemHandler = (TaskHandle_t)pvParameters;

    TEST_MESSAGE("Starting test test_log_single_error_testtask");

    // Log error
    logln_error("Test error");
    char error_str[MAX_ERROR_LOG_STR_SIZE];
    get_most_recent_logged_error(error_str, MAX_ERROR_LOG_STR_SIZE);

    // Compare strings
    TEST_ASSERT_EQUAL_STRING("Test error", error_str);

    // Required at the end of each test job
    end_test_task(xFilesystemHandler);

}
void test_log_single_error_job( void ) {
    TEST_MESSAGE("Starting test test_log_single_error_job");
    test_log_rtos_scheduler_begin(test_log_single_error_testtask);
}


/*
*  Log errros up to ERROR_LOGS_FS_ALLOCATION and check if the most recent errors are retrieved correctly
*/
static void test_log_max_error_testtask( void *pvParameters ) {

    TaskHandle_t xFilesystemHandler = (TaskHandle_t)pvParameters;

    int max_allowed_logged_errors = ERROR_LOGS_FS_ALLOCATION / MAX_ERROR_LOG_STR_SIZE;

    for (int i = 0; i < max_allowed_logged_errors; i++) {
        logln_error("Test error%d", i); // Log error with index

        // Check if the last error is updated correctly
        char error_str[MAX_ERROR_LOG_STR_SIZE];
        get_most_recent_logged_error(error_str, MAX_ERROR_LOG_STR_SIZE);

        // Compare strings
        char expected_error_str[MAX_ERROR_LOG_STR_SIZE];
        sprintf(expected_error_str, "Test error%d", i);
        TEST_ASSERT_EQUAL_STRING(expected_error_str, error_str);
    }

    end_test_task(xFilesystemHandler);

}
void test_log_max_error_job( void ) {
    TEST_MESSAGE("Starting test test_log_max_error_job");
    test_log_rtos_scheduler_begin(test_log_max_error_testtask);
}


/*
* Log errors past the ERROR_LOGS_FS_ALLOCATION and check if the error file's size is being maintained and updated correctly
*/
static void test_log_file_rotation_testtask( void *pvParameters ) {

    TaskHandle_t xFilesystemHandler = (TaskHandle_t)pvParameters;

    int max_allowed_logged_errors = ERROR_LOGS_FS_ALLOCATION / MAX_ERROR_LOG_STR_SIZE;

    // Log max lines allowed * 2 errors to make sure rotation is working
    int expected_file_size = 1; // File size starts at 1
    for (int i = 0; i < max_allowed_logged_errors * 2; i++) {
        char expected_error_str[MAX_ERROR_LOG_STR_SIZE];
        sprintf(expected_error_str, "Test error%d", i % 10); // Only 1 digit to have uniform string size but unique strings
        
        logln_error("%s", expected_error_str); // Log error with index

        // If the error count in the file is < max error messages, add to the length, otherwise, it should not change
        if (i < max_allowed_logged_errors) {
            expected_file_size += strlen(expected_error_str) + 1; // +1 for newline
        }
        // Check file size
        FILINFO file_info;
        stat(ERROR_LOG_FILE_PATH, &file_info);
        
        TEST_ASSERT_EQUAL(expected_file_size, file_info.fsize);

        // Check if the last error is updated correctly
        char error_str[MAX_ERROR_LOG_STR_SIZE];
        get_most_recent_logged_error(error_str, MAX_ERROR_LOG_STR_SIZE);

        // Compare strings
        TEST_ASSERT_EQUAL_STRING(expected_error_str, error_str);
    }

    end_test_task(xFilesystemHandler);

}
void test_log_file_rotation_job( void ) {
    TEST_MESSAGE("Starting test test_log_file_rotation_job");
    test_log_rtos_scheduler_begin(test_log_file_rotation_testtask);
}