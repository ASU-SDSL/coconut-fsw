#pragma once

#include "pico/stdlib.h"
#include "hardware/uart.h"
#include "hardware/irq.h"
#include "pico/stdio/driver.h"

#include "FreeRTOS.h"
#include "queue.h"

#include "command.h"
#include "log.h"

#define UART_MAX_QUEUE_ITEMS 64
#define GSE_CHECK_DELAY_MS 500
#define UART_QUEUE_CHECK_TIME portMAX_DELAY

typedef struct telemetry_queue_transmission {
    uint16_t apid;
    char* payload_buffer;
    size_t payload_size;
} telemetry_queue_transmission_t;

QueueHandle_t uart0_queue;

void gse_queue_message(char* buffer, size_t size);

void uart_initialize(uart_inst_t* uart_instance, int tx_pin, int rx_pin, int irq);

void gse_task();
