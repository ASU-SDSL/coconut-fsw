#pragma once

#include <stdint.h>
#include "pico/stdlib.h"
#include <FreeRTOS.h>
#include "task.h"

#ifdef __cplusplus
extern "C" {
#endif

extern const uint8_t DS18B_U100[8]; 
extern const uint8_t DS18B_U102[8];
extern const uint8_t DS18B_U104[8];

uint8_t ds18b_read_temp(float* data);
// void debug_sample_loop(); // delete 
uint8_t ds18b_start_conversion(); 
float ds18b_get_temp(const uint8_t* addr); 
int16_t ds18b_get_temp_raw(const uint8_t* addr); 

#ifdef __cplusplus
}
#endif