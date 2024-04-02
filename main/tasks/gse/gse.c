#include <stdio.h>
#include "pico/stdlib.h"
#include "sd_card.h"
#include "ff.h"
#include "gse.h"
#include "command.h"
#include "rtc.h"
#include "sdcard.h"
#include "mram.h"

#include "eps.h"
#include "mag.h"

void uart_queue_message(char* buffer, size_t size) {
    // Create new transmission structure
    telemetry_queue_transmission_t new_buffer;
    new_buffer.payload_size = size;
    // Allocate chunk on heap to copy buffer contents
    char* heap_buf = pvPortMalloc(size);
    memcpy(heap_buf, buffer, size);
    new_buffer.payload_buffer = heap_buf;
    // Wait for queue to become available
    while (uart0_queue == NULL) {
        vTaskDelay(GSE_CHECK_DELAY_MS / portTICK_PERIOD_MS);
    }
    xQueueSendToBack(uart0_queue, &new_buffer, portMAX_DELAY);
}

void uart_on_rx() {
    while (uart_is_readable(UART0_INSTANCE)) {
        char ch = uart_getc(UART0_INSTANCE);
        receive_command_byte_from_isr(ch);
    }
}

void uart_initialize(uart_inst_t* uart_instance, int tx_pin, int rx_pin, int irq) {
    // Initialize UART
    uart_init(uart_instance, BAUD_RATE);

    // Set the TX and RX pins
    // TODO: This currently crashes on the feather I think, pins currently setup for the standard Pi Pico
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

void gse_task(void *pvParameters) {
    /* vTaskDelay(2000);


    printf("Get temp\n");
    // uint8_t temp = rtc_test();

    config_i2c0();

    int temp = 0;
    int res = read_temp(i2c0, &temp);
    printf("Result: %d. Temp: %d\n", res, temp);

    // Initialize UART0
    uart_initialize(UART0_INSTANCE, UART0_TX_PIN, UART0_RX_PIN, UART0_IRQ);

    // Create UART0 queue
    // TODO: Change this to a struct instead of char ptr for sending actual command data
    uart0_queue = xQueueCreate(UART_MAX_QUEUE_ITEMS, sizeof(telemetry_queue_transmission_t));

    // Initialize write LED
    gpio_init(LED_PIN);
    gpio_set_dir(LED_PIN, GPIO_OUT);
    
    // Start listening for UART queue messages
    telemetry_queue_transmission_t rec;
    while (true) {
        // Wait on a message in the queue
        xQueueReceive(uart0_queue, &rec, UART_QUEUE_CHECK_TIME);
        // Enable write LED
        gpio_put(LED_PIN, 1);
        // Write bytes to UART
        if (uart_is_writable(UART0_INSTANCE)) {
            // Send bytes to UART port one-by-one
            for (int i = 0; i < rec.payload_size; i++) {
                uart_putc_raw(UART0_INSTANCE, rec.payload_buffer[i]);
            }
        }
        // Free buffer allocated in uart_queue_message
        vPortFree(rec.payload_buffer);
        // Disable write LED
        gpio_put(LED_PIN, 0); */
        mram_testing();
    // }
}
