#pragma once

#include "pico/stdlib.h"
#include "hardware/uart.h"
#include "hardware/irq.h"

#include "FreeRTOS.h"
#include "queue.h"

#include "command.h"
#include "log.h"

#define BAUD_RATE 115200
#define DATA_BITS 8
#define STOP_BITS 1
#define PARITY    UART_PARITY_NONE

#define UART0_INSTANCE uart0
#define UART0_TX_PIN   0
#define UART0_RX_PIN   1

#define LED_PIN PICO_DEFAULT_LED_PIN

#define UART_MAX_QUEUE_ITEMS 64
#define GSE_CHECK_DELAY_MS 500
#define UART_QUEUE_CHECK_TIME portMAX_DELAY

typedef struct telemetry_queue_transmission {
    uint16_t apid;
    char* payload_buffer;
    size_t payload_size;
} telemetry_queue_transmission_t;

QueueHandle_t uart0_queue;

void uart_queue_message(char* buffer, size_t size);

void uart_on_rx();

void uart_initialize(uart_inst_t* uart_instance, int tx_pin, int rx_pin, int irq);

void gse_task();
