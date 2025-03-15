
#include "ds18b_onewire.h"
#include "OneWire.h"


// 16 on breadboard
#define ONE_WIRE_PIN 25
#define MIN_CONVERSION_TIME_MS 1000

#define ONE_WIRE_DEBUG 1

void custom_interrupts_hal(){
    vPortEnterCritical(); 
}

void custom_noInterrupts_hal(){
    vPortExitCritical(); 
}

OneWire ds(ONE_WIRE_PIN, 
            custom_interrupts_hal, 
            custom_noInterrupts_hal);  // on pin 10 (a 4.7K resistor is necessary)

#if ONE_WIRE_DEBUG 
#include <stdio.h>
void debug_sample_loop() {
    uint8_t i;
    uint8_t present = 0;
    uint8_t type_s;
    uint8_t data[9];
    uint8_t addr[8];
    float celsius, fahrenheit;

    if (!ds.search(addr)) {
        printf("No more addresses.");
        printf("\n");
        ds.reset_search();
        busy_wait_ms(250);
        return;
    }

    printf("ROM =");
    for (i = 0; i < 8; i++) {
        printf(" %x", addr[i]);
    }

    if (OneWire::crc8(addr, 7) != addr[7]) {
        printf("CRC is not valid!\n");
        return;
    }

    // the first ROM byte indicates which chip
    switch (addr[0]) {
        case 0x10:
        printf("  Chip = DS18S20\n");  // or old DS1820
        type_s = 1;
        break;
        case 0x28:
        printf("  Chip = DS18B20\n");
        type_s = 0;
        break;
        case 0x22:
        printf("  Chip = DS1822\n");
        type_s = 0;
        break;
        default:
        printf("Device is not a DS18x20 family device.\n");
        return;
    }

    ds.reset();
    ds.select(addr);
    ds.write(0x44, 1);  // start conversion, with parasite power on at the end

    busy_wait_ms(1000);  // maybe 750ms is enough, maybe not

    present = ds.reset();
    ds.select(addr);
    ds.write(0xBE);  // Read Scratchpad

    printf("  Data = ");
    printf("%x", present);
    printf(" ");
    for (i = 0; i < 9; i++) {  // we need 9 bytes
        data[i] = ds.read();
        printf("%x", data[i]);
        printf(" ");
    }
    printf(" CRC=");
    printf("%x", OneWire::crc8(data, 8));
    printf("\n");

    // Convert the data to actual temperature
    // because the result is a 16 bit signed integer, it should
    // be stored to an "int16_t" type, which is always 16 bits
    // even when compiled on a 32 bit processor.
    int16_t raw = (data[1] << 8) | data[0];
    if (type_s) {
        raw = raw << 3;  // 9 bit resolution default
        if (data[7] == 0x10) {
        // "count remain" gives full 12 bit resolution
        raw = (raw & 0xFFF0) + 12 - data[6];
        }
    } else {
        uint8_t cfg = (data[4] & 0x60);
        // at lower res, the low bits are undefined, so let's zero them
        if (cfg == 0x00)
        raw = raw & ~7;  // 9 bit resolution, 93.75 ms
        else if (cfg == 0x20)
        raw = raw & ~3;  // 10 bit res, 187.5 ms
        else if (cfg == 0x40)
        raw = raw & ~1;  // 11 bit res, 375 ms
        //// default is 12 bit resolution, 750 ms conversion time
    }
    celsius = (float)raw / 16.0;
    fahrenheit = celsius * 1.8 + 32.0;
    printf("  Temperature = ");
    printf("%f", celsius);
    printf(" Celsius, ");
    printf("%f", fahrenheit);
    printf(" Fahrenheit\n");
}
#endif 

uint8_t ds18b_read_temp(float* data_out){
    uint8_t i;
    uint8_t present = 0;
    uint8_t type_s;
    uint8_t data[9];
    uint8_t addr[8];
    float celsius;
    
    // search for onewire devices 
    if(!ds.search(addr)){
        ds.reset_search(); 
        vTaskDelay(pdMS_TO_TICKS(250)); 
        // busy_wait_ms(250); 
        return 1; 
    }

    // check crc
    if(OneWire::crc8(addr, 7) != addr[7]) {
        return 2; 
    }

    // make sure the chip is DS18B20 
    if(addr[0] != 0x28){
        return 3; 
    }
    type_s = 0; 

    ds.reset(); 
    ds.select(addr); 
    // not sure if this needs to be 1 for parasitic power, but I don't think it'll break anything 
    ds.write(0x44, 1);  // start conversion, with parasite power on at the end

    vTaskDelay(pdMS_TO_TICKS(1000)); 
    // busy_wait_ms(1000);  // maybe 750ms is enough, maybe not

    present = ds.reset(); 
    ds.select(addr); 
    ds.write(0xBE); // read Scratchpad 

    for (i = 0; i < 9; i++) {  // we need 9 bytes
        data[i] = ds.read();
    }

    int16_t raw = (data[1] << 8) | data[0]; 
    if (type_s) {
        raw = raw << 3;  // 9 bit resolution default
        if (data[7] == 0x10) {
        // "count remain" gives full 12 bit resolution
        raw = (raw & 0xFFF0) + 12 - data[6];
        }
    } else {
        uint8_t cfg = (data[4] & 0x60);
        // at lower res, the low bits are undefined, so let's zero them
        if (cfg == 0x00)
        raw = raw & ~7;  // 9 bit resolution, 93.75 ms
        else if (cfg == 0x20)
        raw = raw & ~3;  // 10 bit res, 187.5 ms
        else if (cfg == 0x40)
        raw = raw & ~1;  // 11 bit res, 375 ms
        //// default is 12 bit resolution, 750 ms conversion time
    }
    celsius = (float)raw / 16.0;

    *data_out = celsius; 

    return 0;
}

uint32_t ds18b_conversion_time = 0;
uint8_t ds18b_start_conversion(){
    // reset OneWire
    ds.reset(); 

    // skip ROM to send to all devices on the bus 
    ds.skip(); 

    // start conversion 
    ds.write(0x44, 1); 

    // record conversion time 
    ds18b_conversion_time = to_ms_since_boot(get_absolute_time());

    return 0; 
}


float ds18b_get_temp(const uint8_t* addr){
    // if it hasn't been long enough since the conversion was started 
    // wait for it to be long enough 
    uint32_t now = to_ms_since_boot(get_absolute_time()); 
    if(now - ds18b_conversion_time < MIN_CONVERSION_TIME_MS){
        vTaskDelay(pdMS_TO_TICKS(MIN_CONVERSION_TIME_MS - (now - ds18b_conversion_time)));
    }

    uint8_t present = ds.reset(); 
    ds.select(addr); 
    ds.write(0xBE); // read Scratchpad

    uint8_t data[9]; 
    for(int i = 0; i < 9; i++){
        data[i] = ds.read(); 
    }

    int16_t raw = (data[1] << 8) | data[0]; 
    
    uint8_t cfg = (data[4] & 0x60);
    // at lower res, the low bits are undefined, so let's zero them
    if (cfg == 0x00)
        raw = raw & ~7;  // 9 bit resolution, 93.75 ms
    else if (cfg == 0x20)
        raw = raw & ~3;  // 10 bit res, 187.5 ms
    else if (cfg == 0x40)
        raw = raw & ~1;  // 11 bit res, 375 ms
    //// default is 12 bit resolution, 750 ms conversion time
    
    float celsius = (float)raw / 16.0;

    return celsius; 
}

