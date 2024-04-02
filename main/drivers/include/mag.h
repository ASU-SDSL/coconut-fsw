#include "hardware/i2c.h"
#include "hardware/gpio.h"
#include <stdio.h>
#include <stdint.h>
#include "log.h"

#include "i2c.h"

/**
 * Configures the Magnetometer for High performance (xyz),
 * data rate of 155Hz, range +/- 4 gauss, and continuous-conversion
 * operation mode. Writes to CTRL registers
*/
int config_mag(i2c_inst_t *i2c);

// x, y, and z outputs can be given units, they are scaled based on config input 
/**
 * Returns the x output (unitless)
*/
int16_t get_x_output(i2c_inst_t *i2c);

/**
 * Returns the y output (unitless)
*/
int16_t get_y_output(i2c_inst_t *i2c);

/**
 * Returns the z output (unitless)
*/
int16_t get_z_output(i2c_inst_t *i2c);

/**
 * Returns the temperature output in Celsius
*/
int16_t get_temp_output(i2c_inst_t *i2c);

/**
 * Returns the value stored in the status register
 * indicated if new data is available, 0 = no new data
 * anything else = new data (datasheet has the breakdown
 * of exactly what new data is available based on the output)
*/
uint8_t get_mag_status(i2c_inst_t *i2c);

int mag_test();