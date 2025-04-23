/**
 * @file vega_ant.h
 * @brief Vega Antenna Board Driver 
 * 
 */

#ifndef VEGA_ANT_H
#define VEGA_ANT_H

#include "hardware/i2c.h"
#include "hardware/gpio.h"
#include <stdint.h>
#include "i2c.h"
#include <FreeRTOS.h>

/**
 * @brief Gets deployment status of antennas 
 * 
 * @param i2c I2C Instance
 * @param output Data Out Buffer 
 * @return uint8_t Status 
 */
uint8_t vega_ant_status(i2c_inst_t* i2c, uint8_t* output);

/**
 * @brief Send command to deploy antennas 
 * 
 * @param i2c I2C Instance
 * @return uint8_t Status 
 */
uint8_t vega_ant_deploy(i2c_inst_t* i2c);

#endif