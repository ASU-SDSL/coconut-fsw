#include "FreeRTOS.h"
#include <string.h>
#include "unity.h"

#include "filesystem.h"
#include "log.h"
#include "heartbeat_job.h"

typedef void (*TaskFunction_t)(void *);

static void test_log_rtos_scheduler_begin(TaskFunction_t task_func_ptr) {
    TaskHandle_t xFilesystemHandler = NULL;

    BaseType_t filesystem_task_status = xTaskCreate(filesystem_task,
                                        "FILESYSTEM",
                                        1024,
                                        NULL,
                                        1,
                                        &xFilesystemHandler);

    BaseType_t testing_task_status = xTaskCreate(task_func_ptr,
                                        "BOOTCOUNT_TEST_TASK",
                                        1024,
                                        xFilesystemHandler, 
                                        1,
                                        NULL); 

    if (filesystem_task_status != pdPASS || testing_task_status != pdPASS) {
        TEST_FAIL_MESSAGE("Task creation failed!");
    }

    vTaskStartScheduler();
    // Should never return
    TEST_FAIL_MESSAGE("Scheduler returned unexpectedly!");
}

static void end_test_task(TaskHandle_t xFilesystemHandler) {
    vTaskEndScheduler();
    vTaskDelete(xFilesystemHandler);
    vTaskDelete(NULL);
}

static void bootcount_test_task(void *pvParameters) {
    TaskHandle_t xFilesystemHandler = (TaskHandle_t)pvParameters;
    uint32_t bootcount_test;

    // --- Test 1: File does not exist ---
    test_bootcount();               
    read_file("boot.bin", (char*)&bootcount_test, sizeof(bootcount_test));
    TEST_MESSAGE("After first test_bootcount call");
    TEST_ASSERT_EQUAL_UINT32(0, bootcount_test);

    // --- Test 2: File exists and can be read ---
    test_bootcount();               
    read_file("boot.bin", (char*)&bootcount_test, sizeof(bootcount_test));
    TEST_MESSAGE("After second test_bootcount call");
    TEST_ASSERT_EQUAL_UINT32(1, bootcount_test);

    end_test_task(xFilesystemHandler);
}

void test_bootcount_job(void) {
    TEST_MESSAGE("Starting test_bootcountjob");
    test_log_rtos_scheduler_begin(bootcount_test_task);
}


int main(void) {
    UNITY_BEGIN();
    
    RUN_TEST(test_bootcount_job);
    
    return UNITY_END();
}