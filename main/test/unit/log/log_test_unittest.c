#include "FreeRTOS.h"

#include "unity.h"

#include "filesystem.h"
#include "log.h"

// Log a single error and check if it is retrieved correctly
static void test_log_single_error_testtask( void *pvParameters ) {

    TEST_MESSAGE("Starting test test_log_single_error_testtask");

    // Log error
    logln_error("Test error");
    char error_str[MAX_ERROR_LOG_STR_SIZE];
    get_most_recent_logged_error(error_str);

    // Compare strings
    TEST_ASSERT_EQUAL_STRING("Test error", error_str);

    // Kill scheduler
    vTaskEndScheduler();
}
void test_log_single_error_job( void ) {

    BaseType_t filesystem_task_status = xTaskCreate(filesystem_task,
                                        "FILESYSTEM",
                                        1024,
                                        NULL,
                                        1,
                                        NULL);

    BaseType_t testing_task_status = xTaskCreate(test_log_single_error_testtask,
                                        "TEST", 
                                        1024, 
                                        NULL, 
                                        1,
                                        NULL); 

    // Start Filesystem scheduler and test task
    vTaskStartScheduler();

    TEST_ASSERT(true); // Make sure the scheduler exits properly
}


// Log errros up to MAX_ERROR_LOG_MESSAGES and check if the most recent errors are retrieved correctly
static void test_log_max_error_testtask( void *pvParameters ) {

    logln_info("Starting task");

    for (int i = 0; i < MAX_ERROR_LOG_MESSAGES; i++) {
        logln_error("Test error%d", i); // Log error with index

        // Check if the last error is updated correctly
        char error_str[MAX_ERROR_LOG_STR_SIZE];
        get_most_recent_logged_error(error_str);

        // Compare strings
        char expected_error_str[MAX_ERROR_LOG_STR_SIZE];
        sprintf(expected_error_str, "Test error%d", i);
        TEST_ASSERT_EQUAL_STRING(expected_error_str, error_str);
    }

    // Kill scheduler
    vTaskEndScheduler();
}
void test_log_max_error_job( void ) {

    TEST_MESSAGE("Starting test test_log_max_error_job");

    BaseType_t filesystem_task_status = xTaskCreate(filesystem_task,
                                        "FILESYSTEM",
                                        1024,
                                        NULL,
                                        1,
                                        NULL);

    BaseType_t testing_task_status = xTaskCreate(test_log_max_error_testtask,
                                        "TEST", 
                                        1024, 
                                        NULL, 
                                        1,
                                        NULL); 

    // Start Filesystem scheduler and test task
    vTaskStartScheduler();
}