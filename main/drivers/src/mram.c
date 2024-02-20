#include "mram.h"

//Slave Address
static uint8_t M24C02_ADDR = 0x50;
static uint8_t DATA_ADDR = 0xAA; //This is where we can start writing memory

//Constants
const int packet_length = 15;
const int max_bytes = 1028;

//Global Modifiable Variables
uint8_t cur_address = DATA_ADDR;


int address_write(i2c_inst_t *i2c, const uint8_t addr, const uint8_t reg, uint8_t* buf, const uint8_t nbytes) {
    int num_bytes_written;
    uint8_t msg[nbytes + 1];

    if (nbytes < 1) {
        return 0;
    }

    msg[0] = reg;
    for  (int i=0; i < nbytes; i++) {
        msg[i + 1] = buf[i];
    }
    num_bytes_written = i2c_write_blocking(i2c, addr, msg, (nbytes + 1), false);

    // this could be an error code as well
    return num_bytes_written;
}

uint8_t* read_bytes(i2c_inst_t * i2c, const uint8_t addr, const uint8_t reg, uint8_t* output_buf, const uint8_t nbytes) {
    int num_bytes_read = 0;
    
    if (nbytes < 1) {
        return 0;
    }

    i2c_write_blocking(i2c, addr, &reg, 1, true);
    num_bytes_read = i2c_read_blocking(i2c, addr, output_buf, nbytes, false);
    return num_bytes_read;
}


void write_byte(i2c_inst_t *i2c, uint8_t byte, uint8_t address) {
    address_write(i2c, byte, address, 1)
}



uint8_t* write_packet(i2c_inst_t *i2c, uint8_t* buf) {
    address_write(i2c, cur_address, DATA_ADDR, output_buf, packet_length);
}

uint8_t* read_packet(i2c_inst_t i2c, uint8_t* output_buf) {
    read_bytes(i2c, cur_address, DATA_ADDR, output_buf, packet_length);
    return 1;
}
