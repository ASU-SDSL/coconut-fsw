#include "radio.h"
#include <stdio.h>
#include "pico/stdlib.h"
#include "hardware/spi.h"

/**
 * @brief Monitor radio, write to SD card, and send stuff when needed
 */


#define BUF_LEN 128   //for length of our buffer testing size
// We need two buffers, one for the data to send, and one for the data to receive.
uint8_t out_buf [BUF_LEN], in_buf [BUF_LEN];

void radio_task(void* unused_arg) {
  // Enable USB serial so we can print
  //stdio_init_all();

  while (true) {
    vTaskDelay(1000);
  }
  
  vTaskDelay(100);
  printf ("SPI Radio Reading\n");
  
  // Enable SPI0 at 1 MHz
  spi_init (spi_default, 1 * 1000000);

  // Assign SPI functions to the default SPI pins
  gpio_set_function (PICO_DEFAULT_SPI_RX_PIN, GPIO_FUNC_SPI);
  gpio_set_function (PICO_DEFAULT_SPI_SCK_PIN, GPIO_FUNC_SPI);
  gpio_set_function (PICO_DEFAULT_SPI_TX_PIN, GPIO_FUNC_SPI);
  gpio_set_function (PICO_DEFAULT_SPI_CSN_PIN, GPIO_FUNC_SPI);

  // // Make the SPI pins available to picotool
  // bi_decl ( bi_4pins_with_func (
  //     PICO_DEFAULT_SPI_RX_PIN,
  //     PICO_DEFAULT_SPI_TX_PIN,
  //     PICO_DEFAULT_SPI_SCK_PIN,
  //     PICO_DEFAULT_SPI_CSN_PIN,
  //     GPIO_FUNC_SPI
  //   )
  // );

  // Initialize the buffers to 0.
  for (u_int8_t i = 0; i < BUF_LEN; ++i) {
    out_buf [i] = 0;
    in_buf [i] = 0;
  }

  int i = 0; //testing purposes
  printf("Radio task is running");
  while (true) {
    printf ("Sending data %d to SPI Peripheral\n", i);
    out_buf [0] = i;
    // Write the output buffer to COPI, and at the same time read from CIPO to the input buffer.
    spi_write_blocking (spi_default, out_buf, 1); //1 for 1 byte communication size
    spi_read_blocking(spi_default, 0, in_buf, 1); //0 for no write buffer, 1 is for expected amnt data
    printf( "Data Received Is: %d\n", in_buf[i]);
    i++;
    vTaskDelay(1000);
  }
  
}