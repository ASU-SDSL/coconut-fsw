#pragma once

// State Structs and Types
typedef enum {
    INIT = 0,
    NOMINAL = 1,
    SAFE = 2
} payload_state_t;

// Global Payload State
payload_state_t g_payload_state;