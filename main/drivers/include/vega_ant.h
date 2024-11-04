#ifndef VEGA_ANT_H
#define VEGA_ANT_H

#include "hardware/i2c.h"
#include "hardware/gpio.h"
#include <stdint.h>
#include "i2c.h"
#include <FreeRTOS.h>


uint8_t vega_ant_status(i2c_inst_t* i2c, uint8_t* output);
uint8_t vega_ant_deploy(i2c_inst_t* i2c);

#endif