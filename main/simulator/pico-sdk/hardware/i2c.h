#pragma once

#include <stdint.h>
#include <stddef.h>
#include <stdbool.h>

#define i2c0 NULL
#define i2c1 NULL
#define PICO_DEFAULT_I2C_SCL_PIN NULL
#define PICO_DEFAULT_I2C_SDA_PIN NULL

typedef unsigned int uint;
typedef struct i2c_inst i2c_inst_t;

uint32_t static i2c_init(i2c_inst_t *i2c, uint32_t baudrate) {
    return baudrate;
}

int static i2c_write_timeout_us(i2c_inst_t *i2c, uint8_t addr, const uint8_t *src, size_t len, bool nostop, uint timeout_us) {
    return 0;
}

// int static i2c_read_from_register(i2c_inst_t *i2c, const uint8_t addr, const uint8_t reg, uint8_t *buf, const uint8_t nbytes)  {
//     return 0;
// }

int static i2c_read_timeout_us(i2c_inst_t *i2c, uint8_t addr, uint8_t *dst, size_t len, bool nostop, uint timeout_us) {
    return 0;
}