#include "ds18b20.h"
#include "onewire_library.h"
#include "log.h"

// code used from pico-examples:
// https://github.com/raspberrypi/pico-examples/tree/master/pio/onewire 

// Function commands for d218b20 1-Wire temperature sensor 
// https://www.analog.com/en/products/ds18b20.html
//
#define DS18B20_CONVERT_T           0x44
#define DS18B20_WRITE_SCRATCHPAD    0x4e
#define DS18B20_READ_SCRATCHPAD     0xbe
#define DS18B20_COPY_SCRATCHPAD     0x48
#define DS18B20_RECALL_EE           0xb8
#define DS18B20_READ_POWER_SUPPLY   0xb4

// Generic ROM commands for 1-Wire devices
// https://www.analog.com/en/technical-articles/guide-to-1wire-communication.html
//
#define OW_READ_ROM         0x33
#define OW_MATCH_ROM        0x55
#define OW_SKIP_ROM         0xCC
#define OW_ALARM_SEARCH     0xEC
#define OW_SEARCH_ROM       0xF0

// pio block config variables 
PIO ds_pio = pio0; 
uint ds_gpio = 16; 
OW ds_ow; 
uint ds_offset; 

#define ONEWIRE_PROGRAM_ADD_FAIL    1
#define ONEWIRE_DRIVER_INIT_FAIL    2
#define ONEWIRE_NO_RESPONSES        3

uint8_t onewire_init(){
    // add the program to the PIO shared address space 
    if(pio_can_add_program(ds_pio, &onewire_program) == false){
        return ONEWIRE_PROGRAM_ADD_FAIL; 
    } 

    ds_offset = pio_add_program(ds_pio, &onewire_program); 

    if(ow_init(&ds_ow, ds_pio, ds_offset, ds_gpio) == false){
        return ONEWIRE_DRIVER_INIT_FAIL;
    }

    return 0; 
}



uint8_t ds18b20_start_conversion(){
    // trigger temperature conversion on all ds18b20 sensors 
    if(ow_reset(&ds_ow) == false){
        return ONEWIRE_NO_RESPONSES; 
    }
    ow_send(&ds_ow, OW_SKIP_ROM); 
    ow_send(&ds_ow, DS18B20_CONVERT_T); 

    return 0; 
}

int16_t ds18b20_read_temp(uint64_t romcode){
    // busy wait until conversion is done 
    while(ow_read(&ds_ow) == 0); 

    // read the result 
    ow_reset(&ds_ow); 
    ow_send(&ds_ow, OW_MATCH_ROM); 
    for(int b = 0; b < 64; b += 8){
        ow_send (&ds_ow, romcode >> b);
    }
    ow_send(&ds_ow, DS18B20_READ_SCRATCHPAD); 
    int16_t temp = 0; 
    temp = ow_read(&ds_ow) | (ow_read(&ds_ow) << 8);

    return temp; 
}

void ds18b20_test(){
    logln_info("Starting ds18b20 test..."); 

    // trigger conversions 
    uint8_t res = ds18b20_start_conversion(); 

    logln_info("DS18B20 Start Conversion res = %d\n", res); 

    vTaskDelay(pdMS_TO_TICKS(750)); // wait for conversion (avoid blocking delay) 

    // read values 
    int16_t u100 = ds18b20_read_temp(DS18B_ROMCODE_U100); 
    int16_t u102 = ds18b20_read_temp(DS18B_ROMCODE_U102); 
    int16_t u104 = ds18b20_read_temp(DS18B_ROMCODE_U104); 

    logln_info("Read Temps u100: %d u102: %d u104: %d (C * 16)", u100, u102, u104); 

    logln_info("Done"); 
}
