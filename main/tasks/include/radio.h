#ifndef RADIO_FILE_DEFINED
#define RADIO_FILE_DEFINED
#include <log.h>
#include <FreeRTOS.h>
#include "queue.h"
#include "command.h"


#define RADIO_NSS_PIN 7
#define RADIO_IRQ_PIN 17
#define RADIO_NRST_PIN 22

#define RADIO_MAX_QUEUE_ITEMS 64


QueueHandle_t radio_queue;

void radio_queue_message(char* buffer, size_t size);
void radio_packet_recieve();
void init_radio();
void radio_task();


#endif /* !RADIO_FILE_DEFINED */