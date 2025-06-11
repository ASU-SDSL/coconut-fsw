/**
 * @file mram.h
 * @brief MRAM Driver
 * 
 */

#pragma once

#include <stdlib.h>
#include "pico/stdlib.h"

//Project specific constants
#define SPI_BUS spi1
#define FREQ 1000000
#define PACKET_SIZE 8
#define MAX_BYTES 32768

// PINS ON Raspberry Pi - to be set by electrical team.
#define SO 8    /// MISO/SDO/SO are all same thing
#define CS 9 
#define SCK 10 
#define SI 11   /// MOSI/SDI/SI are all same thing



#define WP 0 
#define HOLD 0
#define GND 0   /// Wired up to random place
#define PS 0    /// Wired up to random place

/// Command codes
#define WREN 0x06
#define WRDI 0x04
#define RDSR 0x05
#define WRSR 0x01
#define READ 0x03
#define WRITE 0x02
#define SLEEP 0xB9
#define WAKE 0xAB

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

