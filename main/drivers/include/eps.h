#pragma once

#include "hardware/i2c.h"
#include "hardware/gpio.h"

#include "log.h"
#include <stdio.h>
#include <stdint.h>

#include "i2c.h"

/*
* See https://github.com/adafruit/Adafruit_INA219/ for reference
*/


/*
* Calibrates the EPS by writing to the calibration register (0x05)
* Currently set up for a max current of 0.5A
* Returns status of operation (0 = success)
*/
int calibrate(i2c_inst_t *i2c);

/*
* Configures the EPS by writing to the configuration register (0x00)
* Config register value taken from Adafruit_INA219 library
*/
int config(i2c_inst_t *i2c);

/*
* Reads from the VShunt register [reg_vs](0x01) of the device with 
* i2c address [addr] and writes it to [output_buf] in mV
* Returns status of operation (0 = success)
*/
int getVShunt(i2c_inst_t *i2c,
                const uint8_t addr,
                const uint8_t reg_vs,
                float *output_buf);

/*
* Reads from the VShunt register(0x01), 
* writes it to [output_buf] unprocessed
* Returns status of operation (0 = success)
*/
int getVShunt_raw(i2c_inst_t *i2c,
                    uint16_t *output_buf);

/*
* Reads from VBus register [reg_vb](0x02) of the device with 
* i2c address [addr] and writes it to [output_buf] in Volts
* Returns status of operation (0 = success)
*/
int getVBus(i2c_inst_t *i2c,
            const uint8_t addr,
            const uint8_t reg_vb,
            float *output_buf);

/*
* Reads from VBus register (0x02),
* writes it to [output_buf] unprocessed
* Returns status of operation (0 = success)
*/
int getVBus_raw(i2c_inst_t *i2c,
                uint16_t *output_buf);

/*
* Reads from Power register [reg_p](0x03) of the device with 
* i2c address [addr] and writes it to [output_bef] in Watts
* Returns status of operation (0 = success)
*/
int getPower(i2c_inst_t *i2c,
                const uint8_t addr,
                const uint8_t reg_p,
                double *output_buf);

/*
* Reads from Power register (0x03),
* writes it to [output_bef] unprocessed
* Returns status of operation (0 = success)
*/
int getPower_raw(i2c_inst_t *i2c,
                    uint16_t *output_buf);

/*
* Reads from Current register [reg_c](0x04) of the device with 
* i2c address [addr] and writes it to [output_buf] in Amps
* Returns status of operation (0 = success)
*/
int getCurrent(i2c_inst_t *i2c,
                const uint8_t addr,
                const uint8_t reg_c,
                double *output_buf);

/*
* Reads from Current register (0x04)
* writes it to [output_buf] unprocessed
* Returns status of operation (0 = success)
*/
int getCurrent_raw(i2c_inst_t *i2c,
                    uint16_t *output_buf);
                        
/*
* Tests EPS functions
*/
void eps_test();