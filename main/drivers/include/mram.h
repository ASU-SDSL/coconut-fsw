/**
 * @file mram.h
 * @brief MRAM Driver
 * 
 */

#pragma once

#include <stdlib.h>
#include "pico/stdlib.h"


/**
 * @brief Set up MRAM device 
 * 
 */
void setup();

void initialize_mram();

/**
 * @brief Send a simple spi command to MRAM device 
 * 
 * @param cmd Command Byte
 */
void send_simple_command(uint8_t cmd);

/**
 * @brief Writes bytes to the MRAM device
 * 
 * @param addr Address in memory 
 * @param buf Data In Buffer
 * @param nbytes Length of Data In Buffer
 * @return int Status 
 */
int write_bytes(const uint32_t addr, const uint8_t* buf, const uint32_t nbytes);

/**
 * @brief Read bytes from the MRAM device
 * 
 * @param addr Address in memory 
 * @param buf Data In Buffer
 * @param nbytes Length of Data In Buffer
 * @return int Status 
 */
int read_bytes(const uint32_t addr, uint8_t* buf, const uint32_t nbytes);

/**
 * @brief Tests functions in mram.h 
 * 
 */
void mram_testing();

void mram_more_testing();