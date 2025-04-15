#pragma once

#include "pico/stdlib.h"
#include "pico/binary_info.h"
#include <stdint.h>

/**
 * @brief Initializes the PIO block to be a OneWire bus 
 * 
 * @return uint8_t Status code 
 * 0 -> success
 * 1 -> ONEWIRE_PROGRAM_ADD_FAIL
 * 2 -> ONEWIRE_DRIVER_INIT_FAIL
 */
uint8_t onewire_init();

/**
 * @brief Triggers a temperature conversion on all ds18b20 devices on the 
 * onewire line. This can take up to 750ms 
 * 
 * @return uint8_t Status code 
 * 0 -> success
 * 3 -> ONEWIRE_NO_RESPONSES 
 */
uint8_t ds18b20_start_conversion(); 

/**
 * @brief Reads the temperature from a ds18b20 
 * 
 * @param romcode ROM code (64-bit address) to read from 
 * @return int16_t The temperature in C * 16
 */
int16_t ds18b20_read_temp(uint64_t romcode); 

