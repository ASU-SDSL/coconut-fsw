/**
 * @file watchdog.h
 * @brief Watchdog management functions
 */
#pragma once

#include "FreeRTOS.h"
#include <task.h>
#include <semphr.h>
#include <stdbool.h>

/// @brief Stores instance of this task, assigned in main.c and used for notifications (similar to binary semaphores)
TaskHandle_t xWatchdogTaskHandler;

/// @brief Watchdog table mutex 
SemaphoreHandle_t watchlist_mutex; 

/// @brief Struct used for naming task heartbeats for watchdog task to watch 
typedef struct {
    const char* name; 
    volatile bool tick; 
} task_heartbeat_t; 

/**
 * @brief Ticks the given heartbeat 
 * 
 * @param watchdog Pointer to task heartbeat struct 
 */
void task_heartbeat_tick(task_heartbeat_t* watchdog);

/**
 * @brief Creates a task heartbeat struct and saves it to the watchlist 
 * 
 * @param name Name of the task heartbeat struct for debug 
 * @return task_heartbeat_t* Pointer to the new task heartbeat struct 
 */
task_heartbeat_t* build_task_heartbeat(const char* name);

/**
*  @brief Task that controls the resetting of the hardware watchdog - MAX706RESA - this module needs to be reset (GPIO toggled) within 1.6s with 100ns minimum pulse
*  Any task can send this task a signal to stop reseting the watchdog, causing a simple processor reboot
*/
void watchdog_freeze();

/**
*  @brief Runs the watchdog reset loop and will stop if given the freeze signal from the watchdog_freeze function
*/
void watchdog_task(void *pvParameters);
