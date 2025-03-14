#pragma once

#include <stdint.h>
#include "pico/stdlib.h"
#include <FreeRTOS.h>
#include "task.h"

#ifdef __cplusplus
extern "C" {
#endif

uint8_t ds18b_read_temp(float* data);
void debug_sample_loop(); // delete 

#ifdef __cplusplus
}
#endif