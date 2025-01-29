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



#endif /* DS18B20PIO_SRC_DS18B20_H_ */
