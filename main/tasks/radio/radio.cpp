#include <RadioLib.h>
#include "RadioLibPiHal.h"
#define ERR_NONE 0
// #include <SX1278.h>
#include <radio.h>
#include <FreeRTOS.h>
#include "command.h"

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

// create a new instance of the HAL class
PicoHal* hal = new PicoHal(SPI_PORT, SPI_MISO, SPI_MOSI, SPI_SCK);

// Add interupt pin // now we can create the radio module
// NSS pin:  17
// DIO0 pin:  21
// RESET pin:  22
// DIO1/BUSY pin:  20 // not sure why this is named differently here? should be busy
SX1268 radio = new Module(hal, RFM_NSS, RFM_DIO0, RFM_RST, RFM_DIO1);


volatile bool packet_recieved = false;
int state = 0;


#ifdef __cplusplus
extern "C"
{
#endif
void radio_queue_message(char *buffer, size_t size)
{
    // Create new transmission structure
    telemetry_queue_transmission_t new_buffer;
    new_buffer.payload_size = size;
    // Allocate chunk on heap to copy buffer contents
    auto heap_buf = static_cast<char *>(pvPortMalloc(size));
    memcpy(heap_buf, buffer, size);
    new_buffer.payload_buffer = heap_buf;
    // Wait for queue to become available
    while (!radio_queue)
    {
        vTaskDelay(GSE_CHECK_DELAY_MS / portTICK_PERIOD_MS);
    }
    xQueueSendToBack(radio_queue, &new_buffer, portMAX_DELAY);
}
#ifdef __cplusplus
}
#endif

void radio_packet_recieve()
{
    packet_recieved = true;
}

void init_radio()
{
    // make on board LED blink to check for function
    gpio_init(LED_PIN_RADIO);
    gpio_set_dir(LED_PIN_RADIO, GPIO_OUT);
    gpio_put(LED_PIN_RADIO, 1);
    sleep_ms(2000);

    // initialize just like with Arduino
    printf("[SX1268] Initializing ... ");
    state = radio.begin((float)434.0, (float)125.0, (uint8_t)9, (uint8_t)7, (uint8_t)RADIOLIB_SX126X_SYNC_WORD_PRIVATE, (int8_t)-9, (uint16_t)8, (float)0.0);
    if (state != RADIOLIB_ERR_NONE) {
        while (true) {
            printf("failed, code %d\n", state);
            hal->delay(1000);
        }
    }
    printf("success!\n");
}

/**
 * @brief Monitor radio, write to SD card, and send stuff when needed
 */
#ifdef __cplusplus
extern "C"
{
#endif
void radio_task(void *unused_arg)
{
    init_radio();
    // radio_queue = xQueueCreate(RADIO_MAX_QUEUE_ITEMS, sizeof(telemetry_queue_transmission_t));
    // telemetry_queue_transmission_t rec;

    while (true)
    {
        // send a packet
        printf("[SX1268] Transmitting packet ... "); 

        state = radio.transmit("Hello World!");
        if (state == RADIOLIB_ERR_NONE) {
            // the packet was successfully transmitted
            printf("success!\n");

            // wait for a second before transmitting again
            hal->delay(1000);
        } else {
            printf("failed, code %d\n", state);
        }
    }
}
#ifdef __cplusplus
}
#endif