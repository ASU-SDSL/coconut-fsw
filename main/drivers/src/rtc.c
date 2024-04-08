#include "rtc.h"
#include "hardware/i2c.h"
#include "pico/stdlib.h"
#include "pico/binary_info.h"
#include "i2c.h"

// RTC I2C address
#define RTC_ADDR 0x68  // Replace with the correct RTC address

#define RTC_CONTROL_REG 0x0E
#define RTC_STATUS_REG 0x0F

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

// give in 24h time
// returns 0 on success
uint8_t rtc_set_time(i2c_inst_t *i2c, uint8_t hour, uint8_t minute, uint8_t second, uint8_t month, uint8_t date, uint8_t year) {
    uint8_t buf; 

    buf = (second / 10 << 4) | (second % 10);
    if(i2c_write_to_register(i2c, RTC_ADDR, RTC_SECONDS_REG, &buf, 1)){
        return 1;
    }

    buf = (minute / 10 << 4) | (minute % 10);
    if(i2c_write_to_register(i2c, RTC_ADDR, RTC_MINUTES_REG, &buf, 1)){
        return 2;
    }
    //    24h mode      tens place      ones place
    buf = (1 << 6) | (hour / 10 << 4) | (hour % 10);
    if(i2c_write_to_register(i2c, RTC_ADDR, RTC_HOURS_REG, &buf, 1)){
        return 3;
    }

    buf = (date / 10 << 4) | (date % 10);
    if(i2c_write_to_register(i2c, RTC_ADDR, RTC_DATE_REG, &buf, 1)){
        return 4;
    }

    buf = (month / 10 << 4) | (month % 10);
    if(i2c_write_to_register(i2c, RTC_ADDR, RTC_MONTH_CENTURY_REG, &buf, 1)){
        return 5;
    }

    buf = (year / 10 << 4) | (year % 10);
    if(i2c_write_to_register(i2c, RTC_ADDR, RTC_YEAR_REG, &buf, 1)){
        return 6;
    }

    return 0;
}

// returns 0 on success 
uint8_t rtc_update_temp(i2c_inst_t *i2c){
    uint8_t buf; 
    if(i2c_read_from_register(i2c, RTC_ADDR, RTC_CONTROL_REG, &buf, 1)){
        return 1;
    }
    buf = buf | (1 << 5); // set control bit 5 - manually remeasure temperature (auto measures every 64 seconds)

    if(i2c_write_to_register(i2c, RTC_ADDR, RTC_CONTROL_REG, &buf, 1)){
        return 1;
    }

    return 0;
}

// returns 0 on success
uint8_t rtc_read_temp(i2c_inst_t *i2c, float* output) {

    uint8_t tempUpper;
    if(i2c_read_from_register(i2c, RTC_ADDR, RTC_TEMP_REG_UPPER, &tempUpper, 1)){
        return 1;
    }
    uint8_t tempLower;
    if(i2c_read_from_register(i2c, RTC_ADDR, RTC_TEMP_REG_LOWER, &tempLower, 1)){
        return 1;
    }

    // printf("Upper: 0x%x Lower 0x%x\n", tempUpper, tempLower);
    // printf("Upper: %d Lower %f\n", ((int)tempUpper), ((0.5) * (1 && (tempLower & 0b10000000)) + (0.25 * (1 && (tempLower & 0b010000000)))));

    //      2'C integer        fixed point decimal 
    *output = ((int)tempUpper) + ((0.5) * (1 && (tempLower & 0b10000000)) + (0.25 * (1 && (tempLower & 0b010000000))));

    return 0;
}

// returns 0 on success
uint8_t rtc_get_second(i2c_inst_t *i2c, uint8_t* output){
    if(i2c_read_from_register(i2c, RTC_ADDR, RTC_SECONDS_REG, output, 1)){
        return 1;
    }
    //           ones place                     tens place
    *output = (*output & 0b00001111) + (10 * (*output >> 4));

    return 0;
}

// returns 0 on success
uint8_t rtc_get_minute(i2c_inst_t *i2c, uint8_t* output){

    if(i2c_read_from_register(i2c, RTC_ADDR, RTC_MINUTES_REG, output, 1)){
        return 1;
    }

    *output = (*output & 0b00001111) + (10 * (*output >> 4));

    return 0;
}

// returns 0 on success
uint8_t rtc_get_hour(i2c_inst_t *i2c, uint8_t* output){
    if(i2c_read_from_register(i2c, RTC_ADDR, RTC_HOURS_REG, output, 1)){
        return 1;
    }
    if(*output & (0b1 << 6)){ // if bit 6 is high then read as 12h hour 
                //   ones place             add ten if ten bit              add 12 if am/pm bit is pm (1 is am)
        *output = ((*output & 0b00001111) + (10 * (1 && (*output & 0b00010000))) + (12 * (1 && ((*output) & 0b00100000)))) % 24;
    }
    else {
            //      ones place              add ten if ten bit          add 20 if 20 bit
        *output = ((*output & 0b00001111) + (10 * (1 && (*output & 0b00010000)) + (20 * (1 && (*output & 0b00100000))))) % 24;
    }

    return 0; 
}

// returns 0 on success
uint8_t rtc_get_date(i2c_inst_t *i2c, uint8_t* output){
    if(i2c_read_from_register(i2c, RTC_ADDR, RTC_DATE_REG, output, 1)){
        return 1;
    }

    *output = (*output & 0b00001111) + (10 * (*output >> 4));

    return 0;
}

// returns 0 on success
uint8_t rtc_get_month(i2c_inst_t *i2c, uint8_t* output){
    if(i2c_read_from_register(i2c, RTC_ADDR, RTC_MONTH_CENTURY_REG, output, 1)){
        return 1;
    }

    *output &= 0b01111111; // remove century bit 
    //      ones place              add ten if ten bit
    *output = (*output & 0b00001111) + (10 * (*output & 0b00010000));

    return 0;
}

// returns 0 on success
uint8_t rtc_get_year(i2c_inst_t *i2c, uint8_t* output){
    if(i2c_read_from_register(i2c, RTC_ADDR, RTC_YEAR_REG, output, 1)){
        return 1;
    }

    *output = (*output & 0b00001111) + (10 * (*output >> 4)); 

    return 0;
}

void rtc_test() {
   
    i2c_inst_t *i2c = i2c0;

    // Setup i2c
    config_i2c0();

    // set time
    rtc_set_time(i2c, 23, 58, 0, 12, 31, 24);
    sleep_ms(100);

    for(int i = 0; i < 100; i++){
        float temp;
        rtc_update_temp(i2c);
        rtc_read_temp(i2c, &temp);
        printf("RTC Temp: %f\n", temp);

        uint8_t hour;
        uint8_t minute;
        uint8_t second;
        uint8_t month; 
        uint8_t date;
        uint8_t year;
        if(rtc_get_hour(i2c, &hour)){
            printf("Hour failed\n");
        }
        if(rtc_get_minute(i2c, &minute)){
            printf("Minute failed\n");
        }
        if(rtc_get_second(i2c, &second)){
            printf("Second failed\n");
        }
        if(rtc_get_month(i2c, &month)){
            printf("Month failed\n");
        }
        if(rtc_get_date(i2c, &date)){
            printf("Date failed\n");
        }
        if(rtc_get_year(i2c, &year)){
            printf("Date failed\n");
        }

        printf("RTC TimeStamp: \n%d:%d:%d %d/%d/%d \n", hour, minute, second, month, date, year);  
        sleep_ms(1000);
    }

}









