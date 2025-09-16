/**
 * @file hp_BH1750.h
 * @brief Driver for the hp BH1750 Ambient Light Sensor
 * 
 */

#pragma once 

#include "hardware/i2c.h"
#include "hardware/gpio.h"

#define BH1750_I2C_ADDR_1 0x23
#define BH1750_I2C_ADDR_2 0x5C

// combine begin and calibrate and start 
uint8_t bh1750_init(i2c_inst_t* i2c, uint8_t addr);

// combine get reading and restart sampling? 
uint8_t bh1750_get_lux(i2c_inst_t* i2c, uint8_t addr, uint8_t* buf);
