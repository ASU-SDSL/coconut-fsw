#pragma once

#include "hardware/i2c.h"

void set_time(i2c_inst_t *i2c, const uint8_t addr, const uint8_t reg,
              uint8_t hour, uint8_t minute, uint8_t second);
int rtc_test();

int read_temp();
