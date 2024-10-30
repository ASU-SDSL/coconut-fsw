#pragma once

// #include <SX1278.h>

#include <FreeRTOS.h>
#include "queue.h"
#include <stdint.h>


// pinout for on breadboard
#define RADIO_SX_NSS_PIN 28
#define RADIO_SX_DIO1_PIN 15
#define RADIO_SX_NRST_PIN 27
#define RADIO_SX_BUSY_PIN 5

#define RADIO_RFM_NSS_PIN 7
#define RADIO_RFM_DIO0_PIN 17
#define RADIO_RFM_NRST_PIN 22
#define RADIO_RFM_DIO1_PIN 26

// pinout for on pcb
// #define RADIO_SX_NSS_PIN 5
// #define RADIO_SX_DIO1_PIN 22
// #define RADIO_SX_NRST_PIN 24
// #define RADIO_SX_BUSY_PIN 23

// #define RADIO_RFM_NSS_PIN 17
// #define RADIO_RFM_DIO0_PIN 27
// #define RADIO_RFM_NRST_PIN 20
// #define RADIO_RFM_DIO1_PIN 29


#define RADIO_MAX_QUEUE_ITEMS 64

QueueHandle_t radio_queue;

typedef enum radio_operation_type {
    TRANSMIT,
    SET_OUTPUT_POWER,
    ENABLE_RFM98,
    ENABLE_SX1268
} radio_operation_type_t;
typedef struct radio_queue_operations {
    radio_operation_type_t operation_type; // What opeartion to do, could be transmit, set power, etc
    uint8_t* data_buffer; // If extra data is needed for this operation, like if the operation is transmit then the data will be the buffer to transmit
    size_t data_size; // Could be 0
} radio_queue_operations_t;

/* C FUNC DECLARATIONS */

#ifdef __cplusplus
extern "C" void _log(const char* str, ...);
extern "C" void parse_radio_packet(uint8_t* packet, size_t packet_size);
#endif

#include "log.h"
#include "command.h"

#ifdef __cplusplus
extern "C"
{
#endif
    void radio_task(void *unused_arg);
    void radio_queue_message(char *buffer, size_t size);
    void radio_set_transmit_power(uint8_t output_power); 
    void radio_set_module(radio_operation_type_t op); 
#ifdef __cplusplus
}
#endif

// void set_power_output(PhysicalLayer* radio_module, int8_t new_dbm); 
void init_radio();
void radio_task_cpp();