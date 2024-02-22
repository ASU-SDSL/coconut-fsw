#include "hardware/spi.h"
#include "hardware/gpio.h"
#include "log.h"
#include <stdio.h>
#include "pico/stdlib.h" //Probably need this


//Project specific constants
#define SPI_BUS spi0
#define FREQ 5000
#define PACKET_SIZE 15

//PINS ON Raspberry Pi
#define CS 1
#define SO 2
#define WP 3
#define SI 5
#define SCK 6
#define HOLD 7
#define GND 4
#define PS 8

//Command codes
const uint8_t WREN = 0x06h;
const uint8_t WRDI = 0x04h;
const uint8_t RDSR = 0x05h;
const uint8_t WRSR = 0x01h;
const uint8_t READ = 0x03h;
const uint8_t WRITE = 0x02h;
const uint8_t SLEEP = 0xB9h;
const uint8_t WAKE = 0xABh;