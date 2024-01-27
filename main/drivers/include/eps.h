#include "hardware/i2c.h"
#include "hardware/gpio.h"
#include "log.h"
#include <stdio.h>
#include <stdint.h>

#include "i2c.h"

/*
* Calibrates the EPS by writing to the calibration register
* Currently set up for a max current of 0.5A
* Returns status of operation (1 = good)
*/
int calibrate(i2c_inst_t *i2c);

/*
* Reads from the VShunt register [reg_vs] of the device with 
* i2c address [addr] and writes it to [output_buf] in Volts?
* Returns status of operation (1 = good)
*/
int getVShunt(i2c_inst_t *i2c,
                const uint8_t addr,
                const uint8_t reg_vs,
                float *output_buf);

/*
* Reads from VBus register [reg_vb] of the device with 
* i2c address [addr] and writes it to [output_buf] in Volts
* Returns status of operation (1 = good)
*/
int getVBus(i2c_inst_t *i2c,
            const uint8_t addr,
            const uint8_t reg_vb,
            float *output_buf);

/*
* Reads from Power register [reg_p] of the device with 
* i2c address [addr] and writes it to [output_bef] in Watts
* Returns status of operation (1 = good)
*/
int getPower(i2c_inst_t *i2c,
                const uint8_t addr,
                const uint8_t reg_p,
                double *output_buf);

/*
* Reads from Current register [reg_c] of the device with 
* i2c address [addr] and writes it to [output_buf] in Amps
* Returns status of operation (1 = good)
*/
int getCurrent(i2c_inst_t *i2c,
                const uint8_t addr,
                const uint8_t reg_c,
                double *output_buf);
                        
/*
* Tests EPS functions
*/
void eps_test();