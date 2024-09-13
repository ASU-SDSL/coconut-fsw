#pragma once

#include "hardware/i2c.h"
#include "hardware/gpio.h"

#include "log.h"

#include <stdio.h>
#include <stdint.h>

#define I2CSpeed 100000 // Default of 100k
#define I2CTimeout_us 1000000 // Timeout for read and writes in micro-seconds

#define I2C0_SDA_GPIO PICO_DEFAULT_I2C_SDA_PIN
#define I2C0_SCL_GPIO PICO_DEFAULT_I2C_SCL_PIN

/*
* Configure i2c0 speed and pins
*/
void config_i2c0();

/* 
* Write [reg] and [buf] to i2c device, [reg] is just inserted before [buf]
* Returns status of operation (0 = good)
*/
int i2c_write_to_register(	i2c_inst_t *i2c,
							const uint8_t addr,
							const uint8_t reg,
							uint8_t *buf,
							const uint8_t nbytes);

/*
* Write [reg] byte to i2c device and then read [nbytes] from it.
* Read bytes are stored in [buf]
* Returns status of operation (0 = good)
*/
int i2c_read_from_register(	i2c_inst_t *i2c,
							const uint8_t addr,
							const uint8_t reg,
							uint8_t *buf,
							const uint8_t nbytes);
