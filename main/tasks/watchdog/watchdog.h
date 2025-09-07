/**
 * @file watchdog.h
 * @brief Watchdog management functions
 */
#pragma once

#include "FreeRTOS.h"
#include <task.h>

/// @brief Stores instance of this task, assigned in main.c and used for notifications (similar to binary semaphores)
TaskHandle_t xWatchdogTaskHandler;

/**
*  @brief Task that controls the resetting of the hardware watchdog - MAX706RESA - this module needs to be reset (GPIO toggled) within 1.6s with 100ns minimum pulse
*  Any task can send this task a signal to stop reseting the watchdog, causing a simple processor reboot
*/
void watchdog_freeze();

/**
*  @brief Runs the watchdog reset loop and will stop if given the freeze signal from the watchdog_freeze function
*/
void watchdog_task(void *pvParameters);

/**
 * @brief Kicks the watchdog associated with the given ID 
 * 
 * @param id ID of watchdog to kick 
 */
void watchdog_intertask_kick(uint8_t id); 
