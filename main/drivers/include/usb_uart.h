#pragma once

#include "pico/stdio.h"

int writebytes_usb(uint8_t *buffer, size_t size);
int readbytes_usb(uint8_t *buffer, size_t size);