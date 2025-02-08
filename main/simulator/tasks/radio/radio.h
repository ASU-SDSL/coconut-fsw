#pragma once
#include <stdint.h>

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