/**
 * @file gse.h
 * @brief Handles hardline communication over USB 
 */
#pragma once

#define UART_MAX_QUEUE_ITEMS 64
#define GSE_CHECK_DELAY_MS 500
#define UART_QUEUE_CHECK_TIME portMAX_DELAY

extern TaskHandle_t xGSETaskHandler;

typedef struct telemetry_queue_transmission {
    uint16_t apid;
    char* payload_buffer;
    size_t payload_size;
} telemetry_queue_transmission_t;

/**
 * @brief Adds a message to the GSE queue for transmission 
 * over the hardline 
 * 
 * @param buffer Message 
 * @param size Length of Message
 */
void gse_queue_message(char* buffer, size_t size);

/**
 * @brief GSE Task main function 
 * 
 * @param pvParameters 
 */
void gse_task(void *pvParameters);
