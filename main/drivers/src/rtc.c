#include "rtc.h"
#include "hardware/i2c.h"
#include "pico/stdlib.h"
#include "pico/binary_info.h"
#include "i2c.h"

// RTC I2C address
#define RTC_ADDR 0x68  // Replace with the correct RTC address

// Register addresses for time data
#define RTC_SECONDS_REG 0x00
#define RTC_MINUTES_REG 0x01
#define RTC_HOURS_REG 0x02

#define RTC_TEMP_REG_UPPER 0x11
#define RTC_TEMP_REG_LOWER 0x12 // decimal part of the temp

void set_time(i2c_inst_t *i2c, const uint8_t addr, const uint8_t reg,
              uint8_t hour, uint8_t minute, uint8_t second) {
    uint8_t time_data[3];

    // Construct the data to be written to the RTC
    time_data[0] = (second / 10 << 4) | (second % 10);  // Convert decimal to BCD
    time_data[1] = (minute / 10 << 4) | (minute % 10);
    time_data[2] = (hour / 10 << 4) | (hour % 10);

    // Write the time data to the specified register on the RTC
    // @TODO - rewrite using i2c.h functions
    i2c_write_blocking(i2c, addr, &reg, 1, true);  // Set register pointer
    i2c_write_blocking(i2c, addr, time_data, 3, false);
}

// Needs to be tested
int read_temp(i2c_inst_t *i2c, uint8_t* out) {

    /*uint8_t temp_h = 0;
    i2c_write_blocking(i2c, RTC_ADDR, RTC_TEMP_REG_UPPER, 1, false);
    i2c_read_blocking(i2c, RTC_ADDR, &temp_h, 1, false);
    *out = temp_h;*/
    
    uint8_t temp_h = 0;
    if (i2c_read_from_register(i2c, RTC_ADDR, RTC_TEMP_REG_UPPER, &temp_h, 1) == 0) {
        *out = temp_h;
        return 0;
    }

    return 1; // error

}


int rtc_test() {
   
    // Ports
    i2c_inst_t *i2c = i2c0;
    i2c_init(i2c, 100 * 1000);

    gpio_set_function(PICO_DEFAULT_I2C_SDA_PIN, GPIO_FUNC_I2C);
    gpio_set_function(PICO_DEFAULT_I2C_SCL_PIN, GPIO_FUNC_I2C);
    bi_decl(bi_2pins_with_func(PICO_DEFAULT_I2C_SDA_PIN, PICO_DEFAULT_I2C_SCL_PIN, GPIO_FUNC_I2C));

    // Set the time (Example: 5:06:50)
    //set_time(i2c, RTC_ADDR, RTC_HOURS_REG, 5, 6, 50);

    uint8_t temp = 0;
    uint8_t temp_h = read_temp(i2c, &temp);

    // Close the I2C communication
    i2c_deinit(i2c);
    return temp_h;
}









