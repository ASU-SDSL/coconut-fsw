// header file for one wire device
#pragma once

#include <byteswap.h>
#include <stdio.h>
#include <stdlib.h>
#include "pico/stdlib.h"
#include "hardware/gpio.h"
#include "log.h"

#define SERIAL_NUM 64
#define ROM_CODE 64
#define WRITE_SCRATCHPAD 0x4E
#define READ_SCRATCHPAD 0x4E
#define COPY_SCRATCHPAD 0x48
#define CONVERT 0x44
#define RECALL 0xB8
#define READ_POWER 0xB4
#define COMMUNICATION_PIN 0 //add number later

/*
 * DS18B20.h
 * Based on code from
 * Harry Fairhead (2021): "The Pico In C: A 1-Wire PIO Program"
 *  Created on: 25 Mar 2022
 *      Author: jondurrant
 */

#ifndef DS18B20PIO_SRC_DS18B20_H_
#define DS18B20PIO_SRC_DS18B20_H_

#include "pico/stdlib.h"
#include "hardware/gpio.h"
#include "DS1820.pio.h"
#include <stdint.h>

class DS18B20 {
public:
	/***
	 * Constructor, does not initialse and DS18Initialize must also be called
	 */
	DS18B20();

	/***
	 * Constructure auto initialises
	 * @param p
	 * @param gp
	 */
	DS18B20(PIO p, uint8_t gp);

	virtual ~DS18B20();

	/***
	 * Initialise the DS18B20
	 * @param pio
	 * @param gpio
	 * @return
	 */
	void DS18Initalize(PIO p, int gpio);

	/***
	 * Covert Temperature
	 * Leave 1000ms before getting temperature
	 */
	void convert();

	/***
	 * Get term in Celsius. Covert should be called 1000ms before
	 * @param pio
	 * @param sm
	 * @return
	 */
	float getTemperature();

private:
	/***
	 * Read bytes
	 * @param bytes
	 * @param len
	 */
	void readBytes(uint8_t bytes[],  int len);

	/***
	 * Write bytes to PIO
	 * @param bytes
	 * @param len
	 */
	void writeBytes(uint8_t bytes[], int len);

	/***
	 * Return CRC8 of the data
	 */
	uint8_t crc8(uint8_t *data, uint8_t len);

	PIO pio;
	uint sm;
	uint8_t gp;
};

#endif /* DS18B20PIO_SRC_DS18B20_H_ */
