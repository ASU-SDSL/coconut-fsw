#include <iostream>

#include "pico/stdlib.h"
#include "hardware/gpio.h"
#include "modules/pico-onewire/api/one_wire.h"

#include "log.h"

#define TEMP_SENSE_GPIO_PIN 25

extern "C" float read_temp() {
    
    One_wire one_wire(TEMP_SENSE_GPIO_PIN);
	one_wire.init();
	gpio_init(EXIT_GPIO_PIN);
	sleep_ms(1);

    int count = one_wire.find_and_count_devices_on_bus();
    rom_address_t null_address{};
    one_wire.convert_temperature(null_address, true, true);
    for (int i = 0; i < count; i++) {
        auto address = One_wire::get_address(i);
        logln_info("%016llX\t%3.1f*C\r\n", One_wire::to_uint64(address), one_wire.temperature(address));
    }

}