 #pragma once

#include "hardware/spi.h"
#include "hardware/gpio.h"
#include "log.h"
#include "pico/stdlib.h" //Probably need this
#include <stdio.h>
#include <stdlib.h> 
#include <byteswap.h>

//Project specific constants
#define SPI_BUS spi1
#define FREQ 1000000
#define PACKET_SIZE 8
#define MAX_BYTES 32768

//PINS ON Raspberry Pi - to be set by electrical team.
#define CS 9 
#define SO 8 // MISO/SDO/SO are all same thing
#define WP 3 
#define SI 11 // MOSI/SDI/SI are all same thing
#define SCK 10 
#define HOLD 7 // Wired up to power supply
#define GND 4 //Wired up to random place
#define PS 8 // Wired up to random place

//Command codes
#define WREN 0x06
#define WRDI 0x04
#define RDSR 0x05
#define WRSR 0x01
#define READ 0x03
#define WRITE 0x02
#define SLEEP 0xB9
#define WAKE 0xAB


void setup();
void send_simple_command(uint8_t cmd);
int address_write(const uint16_t addr, uint8_t* buf, const uint8_t nbytes);
int read_bytes(const uint16_t addr, uint8_t* buf, const uint8_t nbytes);
int write_packet(uint8_t* buf);
int read_packets(uint8_t* buf, int num_packets);
void mram_testing();

