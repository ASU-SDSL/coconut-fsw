/**
 * @file max17048.h
 * @brief MAX17048 Driver 
 * 
 */

#pragma once

#include "i2c.h"

// Battery Gauge Sensor

// Error codes
#define MAX17048_READ_ERROR -1
#define MAX17048_DEVICE_NOT_READY_ERROR -2
#define MAX17048_VALUE_READ_ERROR -3
#define MAX17048_RESET_ERROR -4
#define MAX17048_WRITE_ERROR -5

/**
 * @brief Turn on IC and bring out of sleep mode 
 * 
 * @param i2c I2C Instance
 * @return int Status 
 */
int max17048Wake(i2c_inst_t *i2c);

/**
 * @brief Read voltage of battery 
 * 
 * @param i2c I2C Instance 
 * @param voltage_out Data Output
 * @return int Status 
 */
int max17048CellVoltage(i2c_inst_t *i2c, float *voltage_out);

/**
 * @brief Read voltage of battery
 * 
 * @param i2c I2C Instance 
 * @param voltage_out Data Output
 * @return int Status 
 */
int max17048CellVoltage(i2c_inst_t *i2c, float *voltage_out);

/**
 * @brief For use in a packet; takes up less bytes; conversion done on the ground
 * Returns 2 uint8_ts
 * 
 * @param i2c I2C Instance
 * @param voltage_out Data Output
 * @return int Status 
 */
int max17048CellVoltageRaw(i2c_inst_t *i2c, uint8_t *voltage_out);

/**
 * @brief Get cell percentage 
 * 
 * @param i2c I2C Instance
 * @param percentage_out Data Output 
 * @return int Status 
 */
int max17048CellPercentage(i2c_inst_t *i2c, float *percentage_out);

/**
 * @brief Get cell percentage raw (2 bytes)
 * 
 * @param i2c I2C Instance
 * @param percentage_out Data Output 
 * @return int Status 
 */
int max17048CellPercentageRaw(i2c_inst_t *i2c, uint8_t *percentage_out);
