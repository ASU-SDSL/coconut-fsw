#include "hardware/i2c.h"

void set_time(i2c_inst_t *i2c, const uint8_t addr, const uint8_t reg,
              uint8_t hour, uint8_t minute, uint8_t second);

int rtc_get_second(i2c_inst_t *i2c);
int rtc_get_minute(i2c_inst_t *i2c);
int rtc_get_hour(i2c_inst_t *i2c);
int rtc_get_date(i2c_inst_t *i2c);
int rtc_get_month(i2c_inst_t *i2c);
int rtc_get_year(i2c_inst_t *i2c);


int read_temp(i2c_inst_t *i2c);

int rtc_get_time(i2c_inst_t *i2c);

int rtc_test();

