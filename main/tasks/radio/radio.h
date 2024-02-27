#ifndef RADIO_FILE_DEFINED
#define RADIO_FILE_DEFINED
#include <FreeRTOS.h>
#include "queue.h"


// LED PIN
#define LED_PIN_RADIO 25

// define pins to be used
#define SPI_PORT spi0
#define SPI_MISO 16 // changed from 4 to 16
#define SPI_MOSI 19 // changed from 3 to 19
#define SPI_SCK 18 // changed from 18 to 2

#define RFM_NSS 17 // changed
#define RFM_RST 22 // changed
#define RFM_DIO0 21 //changed 
#define RFM_DIO1 20 // changed. should be busy pin?

// was here before
#define RADIO_MAX_QUEUE_ITEMS 64

QueueHandle_t radio_queue;

#ifdef __cplusplus
extern "C"
{
#endif
    void radio_queue_message(char *buffer, size_t size);
#ifdef __cplusplus
}
#endif

void radio_packet_recieve();
void init_radio();
void radio_task();


#endif /* !RADIO_FILE_DEFINED */