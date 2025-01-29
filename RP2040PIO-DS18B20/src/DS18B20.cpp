/*
 * DS18B20.cpp
 * Based on code from
 * Harry Fairhead (2021): "The Pico In C: A 1-Wire PIO Program"
 *  Created on: 25 Mar 2022
 *      Author: jondurrant
 */

#include "DS18B20.h"



/***
 * Return CRC8 of the data
 */
uint8_t crc8(uint8_t *data, uint8_t len){
    uint8_t i;
    uint8_t j;
    uint8_t temp;
    uint8_t databyte;
    uint8_t crc = 0;
    for (i = 0; i < len; i++)
    {
        databyte = data[i];
        for (j = 0; j < 8; j++)
        {
            temp = (crc ^ databyte) & 0x01;
            crc >>= 1;
            if (temp)
                crc ^= 0x8C;
            databyte >>= 1;
        }
    }
    return crc;
}

/***
 * Write bytes to PIO
 * @param bytes
 * @param len
 */
void writeBytes(uint8_t bytes[], int len, uint sm, PIO pio){ // sm, pio
    pio_sm_put_blocking(pio, sm, 250);
    pio_sm_put_blocking(pio, sm, len - 1);
    for (int i = 0; i < len; i++)
    {
        pio_sm_put_blocking(pio, sm, bytes[i]);
    }
}

/***
 * Read bytes
 * @param bytes
 * @param len
 */
void readBytes(uint8_t bytes[],  int len, uint sm, PIO pio){ // sm, pio
    pio_sm_put_blocking(pio, sm, 0);
    pio_sm_put_blocking(pio, sm, len - 1);
    for (int i = 0; i < len; i++)
    {
        bytes[i] = pio_sm_get_blocking(pio, sm) >> 24;
    }
}

/***
 * Covert Temperature
 * Leave 1000ms before getting temperature
 */
void convert(){
	uint8_t d[2]= {0xCC, 0x44};
	writeBytes(d, 2);
}

/***
 * Get term in Celsius
 * @return te,[eratire
 */
float getTemperature(){
	uint8_t d[2] = {0xCC, 0xBE};
    writeBytes(d, 2);
    uint8_t data[9];
    readBytes(data, 9);
    uint8_t crc = crc8(data, 9);
    if (crc != 0)
        return -2000;
    int t1 = data[0];
    int t2 = data[1];
    int16_t temp1 = (t2 << 8 | t1);
    volatile float temp = (float)temp1 / 16;
    return temp;
}


/***
 * Initialise the DS18B20
 * @param pio
 * @param gpio
 * @return
 */
void DS18Initalize(PIO p, int gpio){
	pio = p;
	gp = gpio;

    uint offset = pio_add_program(pio, &DS1820_program);
    sm = pio_claim_unused_sm(pio, true);
    pio_gpio_init(pio, gpio);
    pio_sm_config c = DS1820_program_get_default_config(
                                                 offset);
    sm_config_set_clkdiv_int_frac(&c, 255, 0);
    sm_config_set_set_pins(&c, gpio, 1);
    sm_config_set_out_pins(&c, gpio, 1);
    sm_config_set_in_pins(&c, gpio);
    sm_config_set_in_shift(&c, true, true, 8);
    pio_sm_init(pio0, sm, offset, &c);
    pio_sm_set_enabled(pio0, sm, true);
}
