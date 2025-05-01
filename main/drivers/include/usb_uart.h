/**
 * @file usb_uart.h
 * @brief Serial Communication Driver (USB/UART)
 * 
 */
#pragma once

#include <stddef.h>
#include <stdint.h>
#include <stdio.h>

/**
 * @brief Write bytes to USB 
 * 
 * @param buffer Data In Buffer
 * @param size Size of Data In Buffer 
 * @return int Status 
 */
int writebytes_usb(uint8_t *buffer, size_t size);

/**
 * @brief Read bytes from USB 
 * 
 * @param buffer Data Out Buffer 
 * @param size Size of Data Out Buffer 
 * @return int Status 
 */
int readbytes_usb(uint8_t *buffer, size_t size);