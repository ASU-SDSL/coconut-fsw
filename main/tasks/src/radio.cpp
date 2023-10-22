#include <RadioLib.h>
#include "RadioLibPiHal.h"
#define ERR_NONE 0
#include <SX1278.h>

PiPicoHal* hal = new PiPicoHal(spi0, 2000000); // can specify the speed here as an argument if desired
RFM98 radio = new Module(hal, 7, 17, 22, RADIOLIB_NC);

// having issues initializing hal for some reason? 
//I assume it's because of some sort of included path problem to the RadioLib files?

/**
 * @brief Monitor radio, write to SD card, and send stuff when needed
 */
void radio_task(void* unused_arg) {


}

int main() {
    //Create an instance of PiPicoHal
    PiPicoHal hal(spi0, 2000000);

    hal.init();

    while(true) {
        printf("[RFM98] Transmitting packet ...");\
        int state = hal.transmit("Hello World!");
        if (state == 0) {
            printf("success!\n");
            hal.delay(1000);
        } else {
            printf("failed, code %d/n", state);

        }
    }
}
