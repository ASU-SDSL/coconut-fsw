#pragma once

#include <FreeRTOS.h>
#include "queue.h"
#include <stdint.h>


#define RADIO_NSS_PIN 7
#define RADIO_IRQ_PIN 17
#define RADIO_NRST_PIN 22

#define RADIO_MAX_QUEUE_ITEMS 64

QueueHandle_t radio_queue;

typedef enum operation_type {
    TRANSMIT,
    SET_OUTPUT_POWER
} operation_type_t;
typedef struct radio_queue_operations {
    operation_type_t operation_type; // What opeartion to do, could be transmit, set power, etc
    uint8_t* data_buffer; // If extra data is needed for this operation, like if the operation is transmit then the data will be the buffer to transmit
    size_t data_size; // Could be 0
} radio_queue_operations_t;


#ifdef __cplusplus
extern "C"
{
#endif
    void radio_task(void *unused_arg);
    void radio_queue_message(char *buffer, size_t size);
#ifdef __cplusplus
}
#endif

void radio_packet_recieve();
void init_radio();
void radio_task_cpp();