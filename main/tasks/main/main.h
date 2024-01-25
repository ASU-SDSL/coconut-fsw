/**
 * RP2040 FreeRTOS Template
 *
 * @copyright 2022, Tony Smith (@smittytone)
 * @version   1.4.1
 * @license   MIT
 *
 */
#ifndef MAIN_FILE_DEFINED
#define MAIN_FILE_DEFINED


// FreeRTOS
#include <FreeRTOS.h>
#include <task.h>
#include <queue.h>
// C
#include <stdbool.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <time.h>
// Pico SDK
#include "pico/stdlib.h"
#include "pico/binary_info.h"
// Project headers
#include "gse.h"
#include "radio.h"
#include "command.h"
#include "telemetry.h"
#include "log.h"
#include "steve.h"


#ifdef __cplusplus
extern "C" {
#endif

/*
 * GLOBALS
 */

const TickType_t ms_delay = 1000 / portTICK_PERIOD_MS;

/**
 * CONSTANTS
 */
#define         RED_LED_PIN           20


/**
 * PROTOTYPES
 */
void led_task_pico(void* unused_arg);
void led_task_gpio(void* unused_arg);
void log_debug(const char* msg);
void log_device_info(void);


#ifdef __cplusplus
}           // extern "C"
#endif


#endif  /* !MAIN_FILE_DEFINED */
