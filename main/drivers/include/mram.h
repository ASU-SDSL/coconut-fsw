#include "hardware/spi.h"
#include "hardware/gpio.h"
#include "log.h"
#include <stdio.h>
#include "pico/stdlib.h" //Probably need this
//MAKE SURE TO UPDATE CORRESPONDING PINS ON Raspberry Pi
#define CS 1
#define SO 2
#define WP 3
#define GND 4
#define SI 5
#define SCK 6
#define HOLD 7
#define PS 8
