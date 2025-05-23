/**
 * @file ina219.h
 * @brief INA219 Driver 
 * 
 */

#pragma once

#include "hardware/i2c.h"

#define INA0_ADDR 0x40
#define INA1_ADDR 0x41
#define INA2_ADDR 0x42
#define INA3_ADDR 0x43
#define INA4_ADDR 0x44
#define INA5_ADDR 0x45


/*
* See https://github.com/adafruit/Adafruit_INA219/ for reference
*/


/**
 * @brief Calibrates the EPS by writing to the calibration register (0x05)
 * Currently set up for a max current of 0.5A
 * 
 * @param i2c I2C Instance
 * @param addr Device Address
 * @return int status of operation (0 = success)
 */
int calibrate(i2c_inst_t *i2c, const uint8_t addr);

/**
 * @brief Configures the EPS by writing to the configuration register (0x00)
 * Config register value taken from Adafruit_INA219 library
 * 
 * @param i2c I2C Instance
 * @param addr Device Address
 * @return int status of operation (0 = success)
 */
int config(i2c_inst_t *i2c, const uint8_t addr);

/**
 * @brief Reads from the VShunt register [reg_vs](0x01) of the device with 
 * i2c address [addr] and writes it to [output_buf] in mV
 * 
 * @param i2c I2C Instance
 * @param addr Device Address
 * @param reg_vs should be 0x01
 * @param output_buf Data Output Buffer
 * @return int Status of operation (0 = success)
 */
int getVShunt(i2c_inst_t *i2c,
                const uint8_t addr,
                const uint8_t reg_vs,
                float *output_buf);

/**
 * @brief Reads from the VShunt register(0x01), 
 * writes it to [output_buf] unprocessed
 * 
 * @param i2c I2C Instance 
 * @param addr Device Address
 * @param output_buf Data Output Buffer
 * @return int Status of operation (0 = success)
 */
int getVShunt_raw(i2c_inst_t *i2c,
                    const uint8_t addr,
                    uint16_t *output_buf);

/**
 * @brief Reads from VBus register [reg_vb](0x02) of the device with 
 * i2c address [addr] and writes it to [output_buf] in Volts
 * 
 * @param i2c I2C Instance
 * @param addr Device Address
 * @param reg_vb should be 0x02 
 * @param output_buf Data Output Buffer
 * @return int Status of operation (0 = success)
 */
int getVBus(i2c_inst_t *i2c,
            const uint8_t addr,
            const uint8_t reg_vb,
            float *output_buf);

/**
 * @brief Reads from VBus register (0x02),
 * writes it to [output_buf] unprocessed
 * 
 * @param i2c I2C Instance
 * @param addr Device Address
 * @param output_buf Data Output Buffer
 * @return int Status of operation (0 = success)
 */
int getVBus_raw(i2c_inst_t *i2c,
                const uint8_t addr,
                uint16_t *output_buf);

/**
 * @brief Reads from Power register [reg_p](0x03) of the device with 
 * i2c address [addr] and writes it to [output_bef] in Watts
 * 
 * @param i2c I2C Instance
 * @param addr Device Address
 * @param reg_p should be 0x03
 * @param output_buf Data Output Buffer
 * @return int Status of operation (0 = success)
 */
int getPower(i2c_inst_t *i2c,
                const uint8_t addr,
                const uint8_t reg_p,
                double *output_buf);

/**
 * @brief Reads from Power register (0x03),
 * writes it to [output_bef] unprocessed
 * 
 * @param i2c I2C Instance
 * @param addr Device Address
 * @param output_buf Data Output Buffer
 * @return int Status of operation (0 = success)
 */
int getPower_raw(i2c_inst_t *i2c,
                    const uint8_t addr,
                    uint16_t *output_buf);

/**
 * @brief Reads from Current register [reg_c](0x04) of the device with 
 * i2c address [addr] and writes it to [output_buf] in Amps
 * 
 * @param i2c I2C Instance
 * @param addr Device Address
 * @param reg_c should be 0x04
 * @param output_buf Data Output Buffer
 * @return int Status of operation (0 = success)
 */
int getCurrent(i2c_inst_t *i2c,
                const uint8_t addr,
                const uint8_t reg_c,
                double *output_buf);

/**
 * @brief Reads from Current register (0x04)
 * writes it to [output_buf] unprocessed
 * 
 * @param i2c I2C Instance
 * @param addr Device Address
 * @param output_buf Data Output Buffer
 * @return int Status of operation (0 = success)
 */
int getCurrent_raw(i2c_inst_t *i2c,
                    const uint8_t addr,
                    uint16_t *output_buf);
                        
/**
 * @brief Tests EPS functions
 * 
 */
void eps_test();