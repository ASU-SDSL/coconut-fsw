#include "unity.h"

#include "steve.h"

/*
    This test creates a STEVE job and sees if 
    it runs for the correct amount of time
*/

static int count;

static void create_job_testjob( void ) {
    count += 1;
    printf("%d\n", count);
}

static void create_job_testtask( void ) {
    // Wait for steve to become initialized
    vTaskDelay(1000/portTICK_PERIOD_MS);
    // Create steve job
    schedule_recurring_job_secs("UNIT_TEST_JOB", create_job_testjob, 1);
    // Wait at least 5 seconds
    vTaskDelay(6000/portTICK_PERIOD_MS);
    // Kill scheduler
    vTaskEndScheduler();
} 

void test_create_job( void ) {

    BaseType_t scheduler_task_status = xTaskCreate(steve_task, 
                                        "STEVE", 
                                        1024, 
                                        NULL, 
                                        1,
                                        NULL); 

    BaseType_t testing_task_status = xTaskCreate(create_job_testtask, 
                                        "TEST", 
                                        1024, 
                                        &count, 
                                        1,
                                        NULL); 

    // Start STEVE scheduler and test task
    vTaskStartScheduler();

    // Check if count was incremented 5 times in 5 seconds
    TEST_ASSERT_TRUE(count >= 5);
}
