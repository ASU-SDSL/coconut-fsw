#include "hardware/spi.h"
#include "hardware/gpio.h"
#include "log.h"
#include <stdio.h>
#include "pico/stdlib.h" //Probably need this


//Project specific constants
#define SPI_BUS spi0
#define FREQ 5000
#define PACKET_SIZE 8
#define MAX_BYTES 32768

//PINS ON Raspberry Pi - to be set by electrical team.
#define CS 1
#define SO 2
#define WP 3
#define SI 5
#define SCK 6
#define HOLD 7
#define GND 4
#define PS 8

//Command codes
const uint8_t WREN = 0x06;
const uint8_t WRDI = 0x04;
const uint8_t RDSR = 0x05;
const uint8_t WRSR = 0x01;
const uint8_t READ = 0x03;
const uint8_t WRITE = 0x02;
const uint8_t SLEEP = 0xB9;
const uint8_t WAKE = 0xAB;

//TODO: Add method headers for testing right here.
