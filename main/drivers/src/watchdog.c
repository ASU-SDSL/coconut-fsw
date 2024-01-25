#include <stdio.h>
#include <stdint.h>
#include "pico/stdlib.h"    //Include Pico SDK
#include "watchdog.h"

//Function to pet the dog (reset the watchdog timer)
void pet_dog() {
    //Initialize watchdog input and set as output
    gpio_init(WDI_PIN);
    gpio_set_dir(WDI_PIN, GPIO_OUT);

    //Set WDI_PIN to low state
    gpio_put(WDI_PIN, 0);

    //Set WDI_PIN back to high state
    gpio_put(WDI_PIN, 1);

    //Delay for 1 ms before releasing the pin
    //sleep_ms(1)

    //Release WDI pin
    //gpio_put(WDI_PIN, 0);
}

/*
int main() {
    while (1) {
        pet_dog();  
        sleep_ms(1000); //Wait for 1 second before petting the dog again
    }

    return 0;
}
*/