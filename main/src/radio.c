#include "radio.h"
#include <stdio.h>
#include "pico/stdlib.h"
#include "hardware/spi.h"
// #include <RadioLib.h>

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

//task to try transmitting from our RFM96 radio for testing purposes
void transmit_task(void* unused_arg){
  // RFM96 has the following connections: (based on RFM96, might be wrong)
  // NSS/CS pin:   10
  // DIO0 pin:  2
  // RESET pin: 9
  // DIO1 pin:  3
  // RFM96 radio = new Module(10, 2, 9, 3);
  //   Serial.begin(9600);

  //   // initialize RFM96 with default settings
  //   Serial.print(F("[RFM96] Initializing ... "));
  //   int state = radio.begin();
  //   if (state == RADIOLIB_ERR_NONE) {
  //     Serial.println(F("success!"));
  //   } else {
  //     Serial.print(F("failed, code "));
  //     Serial.println(state);
  //     while (true);
  //   }

  //   // some modules have an external RF switch
  //   // controlled via two pins (RX enable, TX enable)
  //   // to enable automatic control of the switch,
  //   // call the following method
  //   // RX enable:   4
  //   // TX enable:   5
  //   /*
  //     radio.setRfSwitchPins(4, 5);
  //   */

  // while(true) {
  //   Serial.print(F("[RFM96] Transmitting packet ... "));

  //   // you can transmit C-string or Arduino string up to
  //   // 256 characters long
  //   // NOTE: transmit() is a blocking method!
  //   //       See example SX127x_Transmit_Interrupt for details
  //   //       on non-blocking transmission method.
  //   int state = radio.transmit("Hello World!");

  //   // you can also transmit byte array up to 256 bytes long
  //   /*
  //     byte byteArr[] = {0x01, 0x23, 0x45, 0x67, 0x89, 0xAB, 0xCD, 0xEF};
  //     int state = radio.transmit(byteArr, 8);
  //   */

  //   if (state == RADIOLIB_ERR_NONE) {
  //     // the packet was successfully transmitted
  //     Serial.println(F(" success!"));

  //     // print measured data rate
  //     Serial.print(F("[RFM96] Datarate:\t"));
  //     Serial.print(radio.getDataRate());
  //     Serial.println(F(" bps"));

  //   } else if (state == RADIOLIB_ERR_PACKET_TOO_LONG) {
  //     // the supplied packet was longer than 256 bytes
  //     Serial.println(F("too long!"));

  //   } else if (state == RADIOLIB_ERR_TX_TIMEOUT) {
  //     // timeout occurred while transmitting packet
  //     Serial.println(F("timeout!"));

  //   } else {
  //     // some other error occurred
  //     Serial.print(F("failed, code "));
  //     Serial.println(state);

  //   }

  //   // wait for a second before transmitting again
  //   vTaskDelay(1000);
  // }
}