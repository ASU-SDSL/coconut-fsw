
#include <stdio.h>
#include "pico/stdlib.h"
#include "DS1820.pio.h"
#include "pico/time.h"
#include "DS18B20.h"
#include "hardware/adc.h"



float getTemp(){
	const float conversion_factor = 3.3f / (1 << 12);
	adc_select_input(4);

	float v = (float)adc_read() * conversion_factor;
	float t = 27.0 - ((v - 0.706)/0.001721);

	return t;
}

int main()
{
	uint32_t time;
	float temp;

	DS18B20 ds = DS18B20(pio0, 7);

    stdio_init_all();
    adc_init();
	adc_set_temp_sensor_enabled(true);

    sleep_ms(3000);
    printf("GO\n");


    for (;;)
    {
    	time = to_ms_since_boot (get_absolute_time ());
    	ds.convert();
    	time = to_ms_since_boot (get_absolute_time ()) - time;
    	printf("Covert time %d ms \n", time);

        sleep_ms(1000);

        time = to_ms_since_boot (get_absolute_time ());
		temp = ds.getTemperature();
        time = to_ms_since_boot (get_absolute_time ()) - time;
        printf("DS18B20 Temp %f in %d ms\r\n", temp, time);

        printf("Pico Temp was %f\n", getTemp());
        sleep_ms(500);
    };
    return 0;
}
