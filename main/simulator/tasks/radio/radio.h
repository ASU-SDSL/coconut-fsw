#pragma once
#include <stdint.h>

typedef enum radio_operation_type {
    TRANSMIT,
    SET_OUTPUT_POWER,
    ENABLE_RFM98,
    ENABLE_SX1268,
    RETURN_STATS,
} radio_operation_type_t;

static void radio_queue_message(char *buffer, size_t size) {return;}

static uint8_t radio_which() {
    return 0; 
}

static uint16_t radio_get_RFM_state() {
    return 0; 
}

static uint16_t radio_get_SX_state() {
    return 0; 
}

static void radio_set_module(radio_operation_type_t op) {
    return;
}

static void radio_set_transmit_power(uint8_t output_power) {
    return;
}

static void radio_queue_stat_response() {
    return;
}
