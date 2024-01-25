#include "hardware/i2c.h"
#include "hardware/gpio.h"
#include <stdio.h>
#include <stdint.h>
#include "log.h"

#include "i2c.h"

int get_x_output(i2c_inst_t *i2c);

int get_y_output(i2c_inst_t *i2c);

int get_z_output(i2c_inst_t *i2c);

int get_temp_output(i2c_inst_t *i2c);

int get_status(i2c_inst_t *i2c);

int mag_test();