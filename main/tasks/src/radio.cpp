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

int transmit(const char* message) {
    int state = radio.begin();
    if (state != ERR_NONE) {
        return state;
    }

    radio.setFrequency(433);
    radio.setTxPower(10);

    state = radio.transmit(message);

    return state;
}


int main() {
    // Initialize the PiPicoHal
    hal->init();

    // Loop forever
    while (true) {
        // Send a packet
        printf("[RFM98] Transmitting packet ... ");
        int state = transmit("Hello World!");
        if (state == ERR_NONE) {
            // The packet was successfully transmitted
            printf("success!\n");
        } else {
            printf("failed, code %d\n", state);
        }

        // Wait for a second before transmitting again
        sleep_ms(1000);
    }

    return 0;
}


// transmit packet using SPI? 
// in need of transmit function using RFM98 logic? use spi_write_blocking to send data over SPI?
// spi_write_blocking(_spi, data, strlen(data)); ??
