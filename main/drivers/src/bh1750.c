/**
 * @file bh1750.c
 * @brief Driver for the HP BH1750 Ambient Light Sensors 
 * based on this Arduino library: https://github.com/Starmbi/hp_BH1750 
 * datasheet: https://www.mouser.com/datasheet/2/348/bh1750fvi-e-186247.pdf 
 * adafruit overview: https://learn.adafruit.com/adafruit-bh1750-ambient-light-sensor/overview 
 */
#include "bh1750.h"

#include "i2c.h"

// combine begin and calibrate and start 
uint8_t bh1750_init(i2c_inst_t* i2c, uint8_t addr){

  return 0; 
}

// combine get reading and restart sampling? 
uint8_t bh1750_get_lux(i2c_inst_t* i2c, uint8_t addr, uint8_t* buf){

  return 0; 
}
