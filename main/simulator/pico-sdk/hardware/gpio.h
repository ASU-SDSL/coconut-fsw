#pragma once

#include <stdint.h>

#define GPIO_OUT 1
#define GPIO_IN 0

enum gpio_function {
    GPIO_FUNC_XIP = 0,
    GPIO_FUNC_SPI = 1,
    GPIO_FUNC_UART = 2,
    GPIO_FUNC_I2C = 3,
    GPIO_FUNC_PWM = 4,
    GPIO_FUNC_SIO = 5,
    GPIO_FUNC_PIO0 = 6,
    GPIO_FUNC_PIO1 = 7,
    GPIO_FUNC_GPCK = 8,
    GPIO_FUNC_USB = 9,
    GPIO_FUNC_NULL = 0xf,
};

void static gpio_init(int gpio) {
    return;
}

void static gpio_set_function(int pin, enum gpio_function func) {
    return;
}

void static gpio_set_dir(uint32_t gpio, int out) {
    return;
}

void static gpio_put(uint32_t gpio, int value) {
    return;
}