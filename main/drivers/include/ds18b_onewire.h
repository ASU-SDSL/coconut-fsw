#pragma once

#include <stdint.h>
#include "pico/stdlib.h"
#include <FreeRTOS.h>
#include "task.h"

#ifdef __cplusplus
extern "C" {
#endif

const uint8_t DS18B_U100[8] = {0x28, 0x0A, 0xAF, 0xD8, 0x0F, 0x00, 0x00, 0xCF};  // 28 A AF D8 F 0 0 CF
const uint8_t DS18B_U102[8] = {0x28, 0x1C, 0xAF, 0xD8, 0x0F, 0x00, 0x00, 0x26};  // 28 1C AF D8 F 0 0 26
const uint8_t DS18B_U104[8] = {0x28, 0x11, 0xAF, 0xD8, 0x0F, 0x00, 0x00, 0x6C};  // 28 11 AF D8 F 0 0 6C

uint8_t ds18b_read_temp(float* data);
// void debug_sample_loop(); // delete 
uint8_t ds18b_start_conversion(); 
float ds18b_get_temp(const uint8_t* addr); 

#ifdef __cplusplus
}
#endif