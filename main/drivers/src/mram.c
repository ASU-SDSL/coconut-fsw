#include "mram.h"

//Global Modifiable Variables
uint16_t cur_addr = 0;

//TODO: Add error codes to methods

void setup() {
    stdio_init_all();
    spi_init(SPI_BUS, FREQ);
    //TODO: See if we need to do anything to change SPI Mode to 0.

    //Setup Serial Pins for SPI
    gpio_set_function(SO, GPIO_FUNC_SPI);
    gpio_set_function(SCK, GPIO_FUNC_SPI);
    gpio_set_function(SI, GPIO_FUNC_SPI);

    //Setup CS pin for SPI
    gpio_init(CS);
    gpio_set_dir(CS, GPIO_OUT); 
    gpio_put(CS, 1);

    //Setup other random pins specific to our device but not necessarily required for SPI
    gpio_set_dir(WP,GPIO_OUT);
    gpio_put(WP, 0);
    gpio_set_dir(HOLD, GPIO_OUT);
    gpio_put(HOLD, 1);

    //Make sure device is initially awake
    send__simple_command(&WAKE);

    //TODO: Set current address pointer using initial space in memory
}

void send__simple_command(uint8_t* cmd) {
    gpio_put(CS, 0);
    spi_write_blocking(SPI_BUS, cmd, 1);
    gpio_put(CS, 1);
}

int address_write(const uint16_t addr, uint8_t* buf, const uint8_t nbytes) {
    send__simple_command(&WREN);
    gpio_put(CS, 0);
    //Trying two methods, send cmd, address, and data bytes separately; Or,make big array for all of them.
    spi_write_blocking(SPI_BUS, &WRITE, 1);
    spi_write_blocking(SPI_BUS, (uint8_t*) &addr, 2); //TODO: Try to change uint16 to an array of uint8s or see if saying uint16 is an array of 2 uint8s works.
    spi_write_blocking(SPI_BUS, buf, nbytes);
    gpio_put(CS, 1);
    send__simple_command(&WRDI);
    //TODO: Update current address after write operation
}

int read_bytes(const uint16_t addr, uint8_t* buf, const uint8_t nbytes) {
    gpio_put(CS, 0);
    spi_write_blocking(SPI_BUS, &READ, 1);
    spi_write_blocking(SPI_BUS, &addr, 2); //TODO: Try to change uint16 to an array of uint8s or see if saying uint16 is an array of 2 uint8s works.
    spi_read_blocking(SPI_BUS, buf, nbytes);
    gpio_put(CS, 1);
}

uint8_t* write_packet(i2c_inst_t *i2c, uint8_t* buf) {
    address_write(cur_addr, buf, PACKET_SIZE);
}

uint8_t* read_packet(i2c_inst_t i2c, uint8_t* buf) {
    read_bytes(cur_addr, buf, PACKET_SIZE); //TODO: Make sure we start from correct address for reading most recent packets
}
