#include "hardware/i2c.h"
#include <stdint.h>

/**
 * Set the RTC's date and time, asumes 24h input. 
 * Returns 0 on success
*/
uint8_t rtc_set_time(i2c_inst_t *i2c, uint8_t hour, uint8_t minute, uint8_t second, uint8_t month, uint8_t date, uint8_t year);

/**
 * ALL GET FUNCTIONS 
 * Reads value from RTC and writes it to [output]
 * Returns 0 on success.
*/
uint8_t rtc_get_second(i2c_inst_t *i2c, uint8_t* output);
uint8_t rtc_get_minute(i2c_inst_t *i2c, uint8_t* output);
uint8_t rtc_get_hour(i2c_inst_t *i2c, uint8_t* output);
uint8_t rtc_get_date(i2c_inst_t *i2c, uint8_t* output);
uint8_t rtc_get_month(i2c_inst_t *i2c, uint8_t* output);
uint8_t rtc_get_year(i2c_inst_t *i2c, uint8_t* output);

/**
 * Reads temp (C) from RTC and writes it to [output] 
*/
uint8_t rtc_read_temp(i2c_inst_t *i2c, float* output);

/**
 * Manually updates value in temp register. RTC auto-updates the temp
 * every 64 seconds.
*/
uint8_t rtc_update_temp(i2c_inst_t *i2c);

/**
 * Test RTC functions
*/
void rtc_test();

