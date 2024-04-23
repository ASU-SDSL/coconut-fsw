#pragma once

#include <stdbool.h>
#include <stdint.h>

#define uart0 NULL
#define UART0_IRQ NULL
#define UART_PARITY_NONE NULL

typedef struct uart_inst uart_inst_t;

int static uart_is_readable (uart_inst_t *uart_instance) {
    return 0;
}

int static uart_is_writeable (uart_inst_t *uart_instance) {
    return 0;
}

int static uart_getc(uart_inst_t *uart_instance) {
    return 'L';
}

void static uart_init(uart_inst_t *uart_instance, int baud_rate) {
    return;
}

void static uart_set_hw_flow(uart_inst_t *uart_instance, int cts, int rts) {
    return;
}

void static uart_set_format(uart_inst_t *uart_instance, int data_bits, int stop_bits, int parity) {
    return;
}

void static uart_set_fifo_enabled(uart_inst_t *uart_instance, int enabled) {
    return;
}

void static uart_set_irq_enables(uart_inst_t *uart, bool rx_has_data, bool tx_needs_data) {
    return;
}

size_t static uart_is_writable(uart_inst_t *uart) {
    return 0;
}

void static uart_putc_raw(uart_inst_t *uart, char c) {
    return;
}