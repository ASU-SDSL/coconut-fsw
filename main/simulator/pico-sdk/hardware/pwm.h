#pragma once

#include "pico/stdlib.h"

enum pwm_chan
{
    PWM_CHAN_A = 0,
    PWM_CHAN_B = 1
};

static void pwm_set_wrap() {
    return;
}

static void pwm_set_chan_level() {
    return;
}

static void pwm_set_enabled() {
    return;
}

static inline uint pwm_gpio_to_slice_num(uint gpio) {
    return 0;
}
