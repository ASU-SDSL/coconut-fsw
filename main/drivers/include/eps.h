#include "hardware/i2c.h"
#include "hardware/gpio.h"
#include "log.h"
#include <stdio.h>

#include "i2c.h"

int getVShunt(        i2c_inst_t *i2c,
                        const uint8_t addr,
                        const uint8_t reg_vs,
                        float *output_buf);

int getVBus(          i2c_inst_t *i2c,
                        const uint8_t addr,
                        const uint8_t reg_vb,
                        float *output_buf);

int getPower(	i2c_inst_t *i2c,
          	 		const uint8_t addr,
	                const uint8_t reg_p,
         	        double *output_buf);

int getCurrent(      i2c_inst_t *i2c,
                        const uint8_t addr,
                        const uint8_t reg_c,
                        double *output_buf);
                        
// tests other functions
int eps_test();