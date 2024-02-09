#include "hardware/i2c.h"
#include "hardware/gpio.h"
#include <stdio.h>
#include <stdint.h>
#include "log.h"

#include "i2c.h"

int16_t get_x_output(i2c_inst_t *i2c);
uint16_t get_x_output_raw(i2c_inst_t *i2c);

int16_t get_y_output(i2c_inst_t *i2c);
uint16_t get_y_output_raw(i2c_inst_t *i2c);

int16_t get_z_output(i2c_inst_t *i2c);
uint16_t get_z_output_raw(i2c_inst_t *i2c);

int get_temp_output(i2c_inst_t *i2c);
uint16_t get_temp_output_raw(i2c_inst_t *i2c);

uint8_t get_mag_status(i2c_inst_t *i2c);

int mag_test();