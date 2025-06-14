/**
 * @file radio.h
 * @brief Handles radio communication 
 * @date 2025-04-22
 * 
 */
#pragma once

// #include <SX1278.h>

#include <FreeRTOS.h>
#include "queue.h"
#include <stdint.h>

// pinout for on breadboard
// #define RADIO_SX_NSS_PIN 28
// #define RADIO_SX_DIO1_PIN 15
// #define RADIO_SX_NRST_PIN 27
// #define RADIO_SX_BUSY_PIN 5

// #define RADIO_RFM_NSS_PIN 7
// #define RADIO_RFM_DIO0_PIN 17
// #define RADIO_RFM_NRST_PIN 22
// #define RADIO_RFM_DIO1_PIN 26

// pinout for on pcb
#define RADIO_SX_NSS_PIN 5
#define RADIO_SX_DIO1_PIN 22
#define RADIO_SX_NRST_PIN 24
#define RADIO_SX_BUSY_PIN 23

#define RADIO_RFM_NSS_PIN 17
#define RADIO_RFM_DIO0_PIN 27
#define RADIO_RFM_NRST_PIN 20
#define RADIO_RFM_DIO1_PIN 29

#define RADIO_FREQ 434.0
#define RADIO_BW 125.0
#define RADIO_SF 9
#define RADIO_CR 7
#define RADIO_SYNC_WORD 18
#define RADIO_PREAMBLE_LEN 8
#define RADIO_RFM_GAIN 0
#define RADIO_SX_TXCO_VOLT 0.0
#define RADIO_SX_USE_REG_LDO false

#define RADIO_MAX_QUEUE_ITEMS 64

#define RADIO_RF_SWITCH_PIN 12
#define RADIO_SX_POWER_PIN 7
#define RADIO_RFM_POWER_PIN 14

// Sets radio switch gpio level to select a radio 
#define RADIO_RF_SWITCH_RFM 1
#define RADIO_RF_SWITCH_SX 0

QueueHandle_t radio_queue;

/// @brief Radio operation types for use in radio_queue_operations_t in the radio_queue
typedef enum radio_operation_type {
    TRANSMIT,
    SET_OUTPUT_POWER,
    ENABLE_RFM98,
    ENABLE_SX1268,
    RETURN_STATS,
} radio_operation_type_t;

/// @brief Radio operation to be added to the radio_queue
typedef struct radio_queue_operations {
    radio_operation_type_t operation_type; ///< What operation does, could be transmit, set power, etc
    uint8_t* data_buffer; ///< If extra data is needed for this operation, for example if the operation is transmit then the data will be the buffer to transmit
    size_t data_size; ///< Could be 0
} radio_queue_operations_t;

/* C FUNC DECLARATIONS */

#ifdef __cplusplus
extern "C" {
    #include "telemetry.h"
    #include "log.h"
    #include "command.h"
}
#endif

#ifdef __cplusplus
extern "C"
{
#endif
    /**
     * @brief C Linkage for radio task function (cpp version is radio_task_cpp()) 
     * 
     * @param unused_arg 
     */
    void radio_task(void *unused_arg);

    /**
     * @brief Adds a message to the radio queue to be transmitted
     * 
     * @param buffer Message to transmit 
     * @param size Size of the message in bytes 
     */
    void radio_queue_message(char *buffer, size_t size);

    /**
     * @brief Changes the transmit power used by the radios
     * The RFM98PW includes and offset of +14dBm to what it is set at
     * so this function will subtract 14 from the given parameter to 
     * accurately produce the output power. This increases the minimum allowed 
     * dBm by 14 as well 
     * 
     * @param output_power New output power in dB
     */
    void radio_set_transmit_power(uint8_t output_power);
    
    /**
     * @brief Adds a change radio module being used operation to the radio queue 
     * 
     * @param op radio_queue_operations_t with operation_type of ENABLE_RFM98 or ENABLE_SX1268
     */
    void radio_set_module(radio_operation_type_t op);
    
    /**
     * @brief Adds a stat response operation to the radio queue
     * Stats Response Operation sends a packet of RSSI, SNR, and Frequency Error
     * 
     */
    void radio_queue_stat_response(); 

    /**
     * @brief Returns which radio is currently set to be used 
     * 
     * @return uint8_t 1 if radio is RFM98, 0 if radio is SX1268
     */
    uint8_t radio_which(); 

    /**
     * @brief Returns the last recorded state from a function called on the RFM98 radio 
     * 
     * @return int16_t RadioLib error code
     */
    int16_t radio_get_RFM_state();
    
    /**
     * @brief Returns the last recorded state from a function called on the SX1268 radio
     * 
     * @return int16_t 
     */
    int16_t radio_get_SX_state(); 
#ifdef __cplusplus
}
#endif

/**
 * @brief Initializes necessary peripherals/settings for the radio task
 * 
 */
void init_radio();

/**
 * @brief The implementation of the radio task function (Linked to C by radio_task())
 * 
 */
void radio_task_cpp();
