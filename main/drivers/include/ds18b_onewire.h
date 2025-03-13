#pragma once

#include <stdint.h>
#include "pico/stdlib.h"
#include <stdio.h>

#ifdef __cplusplus
extern "C" {
#endif

uint8_t ds18b_read_temp(float* data_arr, uint8_t len);
void debug_sample_loop(); // delete 

#ifdef __cplusplus
}
#endif