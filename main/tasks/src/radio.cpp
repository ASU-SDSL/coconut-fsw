#include <RadioLib.h>
#include "RadioLibPiHal.h"
#define ERR_NONE 0
// #include <SX1278.h>
#include <radio.h>
#include <FreeRTOS.h>

PiPicoHal *hal = new PiPicoHal(spi0); // can specify the speed here as an argument if desired
// Add interupt pin
RFM98 radio = new Module(hal, 7, 17, 22, RADIOLIB_NC);
volatile bool packet_recieved = false;

// having issues initializing hal for some reason?
// I assume it's because of some sort of included path problem to the RadioLib files?

void radio_packet_recieve(void)
{
    packet_recieved = true;
}

void init_radio()
{
    PiPicoHal hal(spi0, 2000000);
    hal.init();
    int radio_state = radio.begin();
    if (radio_state == RADIOLIB_ERR_NONE)
    {
        printf("Success, radio initialized");
    }
    else
    {
        printf("failed ");
        while (true)
            ;
    }

    radio.setPacketReceivedAction(radio_packet_recieve);
    int recieve_state = radio.startReceive();
    if (recieve_state == RADIOLIB_ERR_NONE)
    {
        printf("Success, recieving...");
    }
    else
    {
        printf("failed");
        while (true)
            ;
    }
}

/**
 * @brief Monitor radio, write to SD card, and send stuff when needed
 */
void radio_task(void *unused_arg)
{
    init_radio();
    if (packet_recieved){
        uint8_t* packet;
        size_t packet_size = radio.getPacketLength();
        int packet_state = radio.readData(packet, packet_size);
        if(packet_state == RADIOLIB_ERR_NONE){
            printf("Recieved packet");
        } else if (packet_state == RADIOLIB_ERR_CRC_MISMATCH) {
            printf("CRC Error!!");
        } else {
            printf("Packet Reading failed");
        }
    }
}

/**
 * one thread
 * queue polling to check on the size of queue each iteration
 * radio recieve polling
 * On recieve:
 * Send an interrupt when packet recieve https://jgromes.github.io/RadioLib/class_s_x127x.html#ad63322c9c58dd82e4b9982f10e546f33
 * In interupt set a flag -- set the flag to be volitile
 * send to command.c to parse
 * On send:
 * poll the queue
 * if smt in queue send on radio
 */

// int main() {
//     //Create an instance of PiPicoHal
//     PiPicoHal hal(spi0, 2000000);

//     hal.init();

//     while(true) {
//         printf("[RFM98] Transmitting packet ...");
//         int state = radio.transmit("Hello World!");
//         if (state == 0) {
//             printf("success!\n");
//         } else {
//             printf("failed, code %d/n", state);

//         }
//     }
// }
