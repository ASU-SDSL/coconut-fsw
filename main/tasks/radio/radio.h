#pragma once

#include <FreeRTOS.h>
#include "queue.h"
#include <stdint.h>


#define RADIO_SX_NSS_PIN 7
#define RADIO_SX_IRQ_PIN 17
#define RADIO_SX_NRST_PIN 22
#define RADIO_SX_BUSY_PIN 26

#define RADIO_RFM_NSS_PIN 8
#define RADIO_RFM_IRQ_PIN 18
#define RADIO_RFM_NRST_PIN 23

#define RADIO_MAX_QUEUE_ITEMS 64

QueueHandle_t radio_queue;

typedef enum operation_type {
    TRANSMIT,
    SET_OUTPUT_POWER,
    ENABLE_RFM98,
    ENABLE_SX1268
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

void radio_packet_receive();
void init_radio();
void radio_task_cpp();