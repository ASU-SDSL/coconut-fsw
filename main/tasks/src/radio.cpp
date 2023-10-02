#include <RadioLib.h>
#include "RadioLibPiHal.h"

PiPicoHal* hal = new PiPicoHal(); // can specify the speed here as an argument if desired
RFM98 radio = new Module(hal, 7, 17, 22, RADIOLIB_NC);

// having issues initializing hal for some reason? 
//I assume it's because of some sort of included path problem to the RadioLib files?

/**
 * @brief Monitor radio, write to SD card, and send stuff when needed
 */
void radio_task(void* unused_arg) {


}

int main() {
    // Create an instance of PiPicoHal
    PiPicoHal hal(spi0, 2000000);

    // Initialize the PiPicoHal
    hal.init();

    // Loop forever
    while (true) {
        // Send a packet
        printf("[RFM98] Transmitting packet ... ");
        int state = hal.transmit("Hello World!");
        if (state == 0) {
            // The packet was successfully transmitted
            printf("success!\n");

            // Wait for a second before transmitting again
            hal.delay(1000);
        } else {
            printf("failed, code %d\n", state);
        }
    }

    return 0;
}

// transmit packet using SPI? 
// in need of transmit function using RFM98 logic? use spi_write_blocking to send data over SPI?
// spi_write_blocking(_spi, data, strlen(data)); ??
