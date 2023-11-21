// look at eps.c
// copy reg_write and reg_read
#include "rtc.h"
#include "hardware/i2c.h"



void set_time(i2c_inst_t *i2c, const uint8_t addr,const uint8_t reg, 
uint8_t hour, uint8_t minute, uint8_t second) {
    uint8_t time_data[3];
    time_data[0] = second;
    time_data[1] = minute;
    time_data[2] = hour;
    i2c_write_blocking(i2c, addr, time_data, 3, false);
    
    
}
int test(){
    const uint sda_pin = 6;
	const uint scl_pin = 7;

	// Ports
	i2c_inst_t *i2c = i2c0;
    i2c_init(i2c, 400 * 1000);
    
    set_time(i2c, INA219_ADDR, REG_CALIB, 5, 6, 50);
}




/////other code 

#include "rtc.h"
#include "hardware/i2c.h"

// RTC I2C address
static const uint8_t RTC_ADDR = 0x68;  // Replace with the correct RTC address

// Register addresses for time data
static const uint8_t RTC_SECONDS_REG = 0x00;
static const uint8_t RTC_MINUTES_REG = 0x01;
static const uint8_t RTC_HOURS_REG = 0x02;

void set_time(i2c_inst_t *i2c, const uint8_t addr, const uint8_t reg,
              uint8_t hour, uint8_t minute, uint8_t second) {
    uint8_t time_data[3];

    // Construct the data to be written to the RTC
    time_data[0] = (second / 10 << 4) | (second % 10);  // Convert decimal to BCD
    time_data[1] = (minute / 10 << 4) | (minute % 10);
    time_data[2] = (hour / 10 << 4) | (hour % 10);

    // Write the time data to the specified register on the RTC
    i2c_write_blocking(i2c, addr, &reg, 1, true);  // Set register pointer
    i2c_write_blocking(i2c, addr, time_data, 3, false);
}

int test() {
   

    // Ports
    i2c_inst_t *i2c = i2c0;
    i2c_init(i2c, 400 * 1000);

    // Set the time (Example: 5:06:50)
    set_time(i2c, RTC_ADDR, RTC_HOURS_REG, 5, 6, 50);

    // Close the I2C communication
    i2c_deinit(i2c);
}









