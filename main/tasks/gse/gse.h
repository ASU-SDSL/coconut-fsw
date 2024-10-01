#pragma once

#include <string.h>

#include "FreeRTOS.h"
#include "queue.h"

#include "usb_uart.h"
#include "command.h"
#include "rtc.h"
#include "eps.h"
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

void gse_task();
