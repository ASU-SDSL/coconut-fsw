/**
 * @file mag_lis3mdltr.h
 * @brief Magnetometer Driver (LIS3MDLTR)
 * 
 */

#pragma once

#include "hardware/i2c.h"
#include "hardware/gpio.h"

#include <stdint.h>

#include "log.h"
#include "i2c.h"

// Module: LIS3MDLTR

/**
 * @brief Configures the Magnetometer for High performance (xyz),
 * data rate of 155Hz, range +/- 4 gauss, and continuous-conversion
 * operation mode. Writes to CTRL registers
 * 
 * @param i2c I2C Instance 
 * @return int Status (0 = success) 
 */
int mag_config(i2c_inst_t *i2c);

// x, y, and z outputs can be given units, they are scaled based on config input 
/**
 * @brief Writes the x output to [output]
 * 
 * @param i2c I2C Instance 
 * @param output Data Output Buffer
 * @return uint8_t Status (0 = success)
 */
uint8_t mag_get_x(i2c_inst_t *i2c, int16_t* output);

/**
 * @brief Writes the y output to [output]
 * 
 * @param i2c I2C Instance 
 * @param output Data Output Buffer
 * @return uint8_t Status (0 = success)
 */
uint8_t mag_get_y(i2c_inst_t *i2c, int16_t* output);

/**
 * @brief Writes the z output to [output]
 * 
 * @param i2c I2C Instance 
 * @param output Data Output Buffer
 * @return uint8_t Status (0 = success)
 */
uint8_t mag_get_z(i2c_inst_t *i2c, int16_t* output);

/**
 * @brief Writes the temperature output in Celsius to [output]
 * 
 * @param i2c I2C Instance 
 * @param output Data Output Buffer
 * @return uint8_t Status (0 = success)
 */
uint8_t mag_get_temp(i2c_inst_t *i2c, int16_t* output);

/**
 * Returns the value stored in the status register
 * indicated if new data is available, 0 = no new data
 * anything else = new data (datasheet has the breakdown
 * of exactly what new data is available based on the output)
*/

/**
 * @brief Returns the value stored in the status register
 * indicated if new data is available, 0 = no new data
 * anything else = new data (datasheet has the breakdown
 * of exactly what new data is available based on the output)
 * 
 * @param i2c I2C Instance 
 * @return uint8_t Status (see brief)
 */
uint8_t mag_get_status(i2c_inst_t *i2c);

/**
 * @brief Tests the mag_lis3mdltr.h functions 
 * 
 * @return int Status 
 */
int mag_test();
