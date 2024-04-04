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
#define RTC_DAY_REG 0x03
#define RTC_DATE_REG 0x04
#define RTC_MONTH_CENTURY_REG 0x05
#define RTC_YEAR_REG 0x06

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
int read_temp(i2c_inst_t *i2c) {

    /*uint8_t temp_h = 0;
    i2c_write_blocking(i2c, RTC_ADDR, RTC_TEMP_REG_UPPER, 1, false);
    i2c_read_blocking(i2c, RTC_ADDR, &temp_h, 1, false);
    *out = temp_h;*/
    
    // uint8_t temp_h = 0;
    // if (i2c_read_from_register(i2c, RTC_ADDR, RTC_TEMP_REG_UPPER, &temp_h, 1) == 0) {
    //     *out = temp_h;
    //     return 0;
    // }

    return 1; // error

}

int rtc_get_second(i2c_inst_t *i2c){
    uint8_t input;
    printf("%x\n", input);
    i2c_read_from_register(i2c, RTC_ADDR, RTC_SECONDS_REG, &input, 1);
    //      ones place              tens place
    return (int)(input & 0b00001111) + (int)(10 * (input >> 4));
}

int rtc_get_minute(i2c_inst_t *i2c){
    uint8_t input;
    i2c_read_from_register(i2c, RTC_ADDR, RTC_MINUTES_REG, &input, 1);

    return (int)(input & 0b00001111) + (int)(10 * (input >> 4));
}

int rtc_get_hour(i2c_inst_t *i2c){
    uint8_t input;
    i2c_read_from_register(i2c, RTC_ADDR, RTC_HOURS_REG, &input, 1);
    return input; 
}

int rtc_get_date(i2c_inst_t *i2c){
    uint8_t input;
    i2c_read_from_register(i2c, RTC_ADDR, RTC_DATE_REG, &input, 1);

    if(input & (0b1 << 6)){ // if bit 6 is high then read as 12h hour 
            //   ones place             add ten if ten bit              add 12 if am/pm bit is pm (1 is am)
        return (input & 0b00001111) + (10 * (input & 0b00010000)) + (12 * !(input & 0b00100000));
    }
    else {
        //      ones place              add ten if ten bit          add 20 if 20 bit
        return (input & 0b00001111) + (10 * (input & 0b00010000) + (20 * (input & 0b00100000)));
    }

    return input; 
}

int rtc_get_month(i2c_inst_t *i2c){
    uint8_t input;
    i2c_read_from_register(i2c, RTC_ADDR, RTC_MONTH_CENTURY_REG, &input, 1);

    input &= 0b01111111; // remove century bit 
    //      ones place              add ten if ten bit
    return (input & 0b00001111) + (10 * (input & 0b00010000));
}

int rtc_get_year(i2c_inst_t *i2c){
    uint8_t input;
    i2c_read_from_register(i2c, RTC_ADDR, RTC_YEAR_REG, &input, 1);

    return (input & 0b00001111) + (10 * (input & 0b11110000)); 
}

int rtc_test() {
   
    i2c_inst_t *i2c = i2c0;

    // Setup i2c
    config_i2c0();

    // config mag
    config_mag(i2c);

    for(int i = 0; i < 10; i++){

        printf("RTC Temp: %d\n", read_temp(i2c));

        printf("RTC Time: %d:%d:%d\n", rtc_get_hour(i2c), rtc_get_minute(i2c), rtc_get_second(i2c)); 

        sleep_ms(100);
    }

    // // Ports
    // i2c_inst_t *i2c = i2c0;
    // i2c_init(i2c, 100 * 1000);

    // gpio_set_function(PICO_DEFAULT_I2C_SDA_PIN, GPIO_FUNC_I2C);
    // gpio_set_function(PICO_DEFAULT_I2C_SCL_PIN, GPIO_FUNC_I2C);
    // bi_decl(bi_2pins_with_func(PICO_DEFAULT_I2C_SDA_PIN, PICO_DEFAULT_I2C_SCL_PIN, GPIO_FUNC_I2C));

    // // Set the time (Example: 5:06:50)
    // //set_time(i2c, RTC_ADDR, RTC_HOURS_REG, 5, 6, 50);

    // uint8_t temp = 0;
    // uint8_t temp_h = read_temp(i2c, &temp);

    // // Close the I2C communication
    // i2c_deinit(i2c);
    // return temp_h;
}









