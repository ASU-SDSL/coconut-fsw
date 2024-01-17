#include "hardware/i2c.h"
#include "hardware/gpio.h"
#include <stdio.h>
#include <stdint.h>
#include "log.h"

int i2c_driver_write(	i2c_inst_t *i2c,
				const uint8_t addr,
				const uint8_t reg,
				uint8_t *buf,
				const uint8_t nbytes);

int i2c_driver_read(	i2c_inst_t *i2c,
				const uint8_t addr,
				const uint8_t reg,
				uint8_t *buf,
				const uint8_t nbytes);