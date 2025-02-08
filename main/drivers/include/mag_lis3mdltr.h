#pragma once

#include "hardware/i2c.h"
#include "hardware/gpio.h"

#include <stdint.h>

#include "log.h"
#include "i2c.h"

// Module: LIS3MDLTR

/**
 * Configures the Magnetometer for High performance (xyz),
 * data rate of 155Hz, range +/- 4 gauss, and continuous-conversion
 * operation mode. Writes to CTRL registers
*/
int mag_config(i2c_inst_t *i2c);

// x, y, and z outputs can be given units, they are scaled based on config input 
/**
 * Writes the x output to [output]
 * Returns 0 on success
*/
uint8_t mag_get_x(i2c_inst_t *i2c, int16_t* output);

/**
 * Writes the y output to [output]
 * Returns 0 on success
*/
uint8_t mag_get_y(i2c_inst_t *i2c, int16_t* output);

/**
 * Writes the z output to [output]
 * Returns 0 on success
*/
uint8_t mag_get_z(i2c_inst_t *i2c, int16_t* output);

/**
 * Writes the temperature output in Celsius to [output]
 * Returns 0 on success
*/
uint8_t mag_get_temp(i2c_inst_t *i2c, int16_t* output);

/**
 * Returns the value stored in the status register
 * indicated if new data is available, 0 = no new data
 * anything else = new data (datasheet has the breakdown
 * of exactly what new data is available based on the output)
*/
uint8_t get_mag_status(i2c_inst_t *i2c);

int mag_test();
