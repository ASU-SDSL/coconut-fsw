#pragma once

#include "i2c.h"

static int max17048CellVoltage(i2c_inst_t *i2c, float *voltage_out) {
    *voltage_out = 3.33369;
    return 0;
}

int max17048CellPercentage(i2c_inst_t *i2c, float *percentage_out) {
    *percentage_out = 45.0;
    return 0;
}

static int max17048CellPercentageRaw(i2c_inst_t *i2c, uint8_t *percentage_out) {
    *percentage_out = 45;
    return 0;
}

static int max17048Wake(i2c_inst_t *i2c) {
    return 0;
}