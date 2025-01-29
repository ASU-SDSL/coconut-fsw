# RP2040PIO-DS18B20
Raspberry Pi PICO or RP2040 PIO Lib for DS18B20 Temperature Sensor 

Based on code from Harry Fairhead (2021): "The Pico In C: A 1-Wire PIO Program".

## Dependencies

Pico SDK dependencies only (GPIO and PIO)

## Example
See main.cxx

## Include in own projects
Simply include DS18B20pio.cmake in your CMakefile
Then add ds18b20pio into target_link_libaries


## Usage
Construct object: 

	DS18B20 ds = DS18B20(pio0, 7);
Giving the pio being used, I've only used pio0. And the GPIO number, in this case 7.

Trigger the device to sample with the command:

	ds.convert();


Then give the divise approx 1000ms to complete that before reading the temperature with:

	float temp = ds.getTermperature();