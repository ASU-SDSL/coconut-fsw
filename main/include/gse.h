#ifndef GSE_FILE_DEFINED
#define GSE_FILE_DEFINED

#include "pico/stdlib.h"
#include "hardware/uart.h"
#include "hardware/irq.h"

#include <FreeRTOS.h>
#include <queue.h>

#define BAUD_RATE 115200
#define DATA_BITS 8
#define STOP_BITS 1
#define PARITY    UART_PARITY_NONE

#define UART0_INSTANCE uart0
#define UART0_TX_PIN   0
#define UART0_RX_PIN   1

#define UART_MAX_QUEUE_ITEMS  64
#define UART_QUEUE_CHECK_TIME 500
typedef char* uart_message_type;

QueueHandle_t uart0_queue;

void uart_queue_message(uart_message_type message);

void uart_on_rx();

void uart_initialize(uart_inst_t* uart_instance, int tx_pin, int rx_pin, int irq);

void gse_task();

#endif  /* !GSE_FILE_DEFINED */