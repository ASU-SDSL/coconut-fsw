#include "mram.h"
#include "FreeRTOS.h"

#include "hardware/spi.h"
#include "hardware/gpio.h"
#include "log.h"
#include <stdio.h>

//Project specific constants
#define SPI_BUS spi1
#define FREQ 1000000
#define PACKET_SIZE 8
#define MAX_BYTES 32768

// PINS ON Raspberry Pi - to be set by electrical team.
#define SO 8    /// MISO/SDO/SO are all same thing
#define CS 9 
#define SCK 10 
#define SI 11   /// MOSI/SDI/SI are all same thing

#define WP 0 
#define HOLD 0
#define GND 0   /// Wired up to random place
#define PS 0    /// Wired up to random place

/// Command codes
#define WREN 0x06
#define WRDI 0x04
#define RDSR 0x05
#define WRSR 0x01
#define READ 0x03
#define WRITE 0x02
#define SLEEP 0xB9
#define WAKE 0xAB

//Global Modifiable Variables
uint16_t memory_start;
uint16_t cur_addr;

//TODO: Add error codes to methods
//TODO: Finish code to make sure write and read command bytes are all sent at once
//TODO: Add flag to indicate whether the entire storage has already been used.

//Make sure code doesn't break due to copying to local variable in method instead of using exact same memory location. 
void send_simple_command(uint8_t cmd) {
    gpio_put(CS, 0);
    spi_write_blocking(SPI_BUS, &cmd, 1);
    gpio_put(CS, 1);
}

void initialize_mram() {
    spi_init(SPI_BUS, FREQ);

    //Setup Serial Pins for SPI
    gpio_set_function(SO, GPIO_FUNC_SPI);
    gpio_set_function(SCK, GPIO_FUNC_SPI);
    gpio_set_function(SI, GPIO_FUNC_SPI);

    //Setup CS pin for SPI
    gpio_init(CS);
    gpio_set_dir(CS, GPIO_OUT); 
    gpio_put(CS, 1);

    //Make sure device is initially awake
    send_simple_command(WAKE);
}

int write_bytes(uint32_t addr, const uint8_t* buf, const uint32_t nbytes) {
    if (nbytes == 0) { return 0; }

    send_simple_command(WREN);

    uint32_t bendaddr = __builtin_bswap32(addr);
    uint8_t *p_bendaddr = (uint8_t*) &bendaddr;

    uint8_t arr[4] = {WRITE, p_bendaddr[1], p_bendaddr[2], p_bendaddr[3]};
   
    gpio_put(CS, 0);
    spi_write_blocking(SPI_BUS, arr, 4);
    spi_write_blocking(SPI_BUS, buf, nbytes);
    gpio_put(CS, 1);

    send_simple_command(WRDI);
    
    return nbytes;
}

int read_bytes(uint32_t addr, uint8_t* buf, const uint32_t nbytes) {
    if (nbytes == 0) { return 0; }
    
    gpio_put(CS, 0);

    uint32_t bendaddr = __builtin_bswap32(addr);
    uint8_t *p_bendaddr = (uint8_t*) &bendaddr;

    uint8_t arr[4] = {READ, p_bendaddr[1], p_bendaddr[2], p_bendaddr[3]};

    spi_write_blocking(SPI_BUS, arr, 4); 
    spi_read_blocking(SPI_BUS, 0, buf, nbytes);
    gpio_put(CS, 1);
    return nbytes;
}

void mram_testing() {
    initialize_mram();

    while(true) {
        uint8_t my_buf[8] = {1, 9, 8, 4, 256, 33, 22, 1};
        write_bytes(100, my_buf, 8);
        
        //vTaskDelay(500);

        uint8_t output[8] = {0,0,0,0,0,0,0,0};
        read_bytes(100, output, 8);

        printf("Writing: ");
        for (int i = 0; i < 8; i++)
        {
            printf("%d ", my_buf[i]);
        }
        printf("\n");

        printf("Reading: ");
        for (int i = 0; i < 8; i++)
        {
            printf("%d ", output[i]);
        }
        printf("\n");
        vTaskDelay(200);

    }
}
