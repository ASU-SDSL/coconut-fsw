#pragma once

#include <stdint.h>

#include "rtc_ds3231.h"
#include <FreeRTOS.h>

/**
 * @brief Set the real time clock to the given time 
 * 
 * @param args A pointer to an array of uint8_t with the form [year, month, day, hour, minute, second] 
 */
void set_rtc_job(void* args); 