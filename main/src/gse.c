#include "gse.h"
#include "command.h"

void uart_queue_message(uart_message_type message) {
    // Wait for queue to become available
    while (uart0_queue == NULL) {
        vTaskDelay(500);
    }
    xQueueSendToBack(uart0_queue, &message, portMAX_DELAY);
}

void uart_on_rx() {
    while (uart_is_readable(UART0_INSTANCE)) {
        uint8_t ch = uart_getc(UART0_INSTANCE);
        if (uart_is_writable(UART0_INSTANCE)) {
            // uart_putc(UART0_INSTANCE, ch);
        }
    }
}

void uart_initialize(uart_inst_t* uart_instance, int tx_pin, int rx_pin, int irq) {
    // Initialize UART
    uart_init(uart_instance, BAUD_RATE);

    // Set the TX and RX pins
    gpio_set_function(tx_pin, GPIO_FUNC_UART);
    gpio_set_function(rx_pin, GPIO_FUNC_UART);

    // Turn off CTS and RTS
    uart_set_hw_flow(uart_instance, false, false);

    // Set our data format
    uart_set_format(uart_instance, DATA_BITS, STOP_BITS, PARITY);

    // Turn off FIFO's - we want to do this character by character
    uart_set_fifo_enabled(uart_instance, false);

    // Set up and enable the interrupt handler
    irq_set_exclusive_handler(irq, uart_on_rx);
    irq_set_enabled(irq, true);

    // Now enable the UART to send interrupts - RX only
    uart_set_irq_enables(uart_instance, true, false);
}

void gse_task() {
    // Initialize UART0
    uart_initialize(UART0_INSTANCE, UART0_TX_PIN, UART0_RX_PIN, UART0_IRQ);
    
    // Create UART0 queue
    // TODO: Change this to a struct instead of char ptr for sending actual command data
    uart0_queue = xQueueCreate(UART_MAX_QUEUE_ITEMS, sizeof(uart_message_type));

    // Initialize write LED
    const uint LED_PIN = PICO_DEFAULT_LED_PIN;
    gpio_init(LED_PIN);
    gpio_set_dir(LED_PIN, GPIO_OUT);

    // Start listening for UART queue messages
    uart_message_type message = NULL;
    while (true) {
        // Wait on a message in the queue
        xQueueReceive(uart0_queue, &message, UART_QUEUE_CHECK_TIME);
        // Check valid message
        if (message != NULL) {
            // Enable write LED
            gpio_put(LED_PIN, 1);
            // Write bytes to UART
            if (uart_is_writable(UART0_INSTANCE)) {
                uart_puts(UART0_INSTANCE, message);
            }
            // Disable write LED
            gpio_put(LED_PIN, 0);
        }
    }
}