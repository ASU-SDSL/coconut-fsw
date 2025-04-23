#pragma once

#include "pico/stdlib.h"
#include "pico/binary_info.h"
#include <stdint.h>

// eps one wire romcodes 
// addresss for pio library 
// (reversed of what was found bit bang)        // bit banged addresses
#define DS18B_ROMCODE_U100 0xCF00000FD8AF0A28   // 28 0A AF D8 0F 00 00 CF
#define DS18B_ROMCODE_U102 0x2600000FD8AF1C28   // 28 1C AF D8 0F 00 00 26
#define DS18B_ROMCODE_U104 0x6C00000FD8AF1128   // 28 11 AF D8 0F 00 00 6C

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
 * This requires waiting for that conversion to finish, if
 * this function is called and the conversion hasn't finished
 * it will block until it has
 * 
 * @param romcode ROM code (64-bit address) to read from 
 * @return int16_t The temperature in C * 16
 */
int16_t ds18b20_read_temp(uint64_t romcode); 

/**
 * @brief Test function for ds18b20 
 * 
 */
void ds18b20_test(); 

/**
 * @brief More general ds18b20 onewire test, scans for all devices
 * on the onewire bus. The pio config selection used is internal to 
 * this function (pin, pio block, etc.)
 * 
 */
void ds18b20_scan(); 