#include "mram.h"

//Global Modifiable Variables
uint16_t memory_start;
uint16_t cur_addr;

//TODO: Add error codes to methods
//TODO: Use bit-shifting instead of multiplication for uint16 and uint8
//TODO: Add flag to indicate whether the entire storage has already been used.

void setup() {
    stdio_init_all();
    spi_init(SPI_BUS, FREQ);
    //TODO: See if we need to do anything to change SPI Mode to 0.

    //Setup Serial Pins for SPI
    gpio_set_function(SO, GPIO_FUNC_SPI);
    gpio_set_function(SCK, GPIO_FUNC_SPI);
    gpio_set_function(SI, GPIO_FUNC_SPI);
    gpio_set_function(CS, GPIO_FUNC_SPI);

    /* //Setup CS pin for SPI
    gpio_init(CS);
    gpio_set_dir(CS, GPIO_OUT); 
    gpio_put(CS, 1); */

    //Setup other random pins specific to our device but not necessarily required for SPI
    gpio_set_dir(WP,GPIO_OUT);
    gpio_put(WP, 0);
    gpio_set_dir(HOLD, GPIO_OUT);
    gpio_put(HOLD, 1);

    //Make sure device is initially awake
    send__simple_command(WAKE);

    /* //Add padding so that only a whole number of packets will fit into data storage. Padding should also be at least 2 bytes to allow for memory for cur_addr pointer.
    memory_start = MAX_BYTES % PACKET_SIZE;
    if (memory_start < 2) { memory_start += PACKET_SIZE; }
    Documents/coconut/coconut-fsw$
    //Set current address pointer based on whWRENat was last set in the two bytes before the start of packet memory. In case pointer was not set properly, the pointer will default to start of packet memory. 
    uint8_t p[2];
    read_bytes(memory_start - 2, p, 2);
    cur_addr = max(p[0]*0x100 +p[1], memory_start);
    */
}

//Make sure code doesn't break due to copying to local variable in method instead of using exact same memory location. 
void send__simple_command(uint8_t cmd) {
    //gpio_put(CS, 0);
    spi_write_blocking(SPI_BUS, &cmd, 1);
    //gpio_put(CS, 1);
}

int address_write(const uint16_t addr, uint8_t* buf, const uint8_t nbytes) {
     if (nbytes <= 0) { return 0; }

    send__simple_command(WREN);
    //gpio_put(CS, 0);
    //Trying two methods, send cmd, address, and data bytes separately; Or,make big array for all of them.
    uint8_t cmd = WRITE;
    spi_write_blocking(SPI_BUS, &cmd, 1);
    // uint16_t big_endian_addr = ((addr & 0xFF) << 8) | (addr & 0xFF00);
    uint8_t arr[2] = {addr/0x100, addr % 0x100}; // Code to cast uint16 address into two bytes
    spi_write_blocking(SPI_BUS, arr, 2);
    spi_write_blocking(SPI_BUS, buf, nbytes);
    //gpio_put(CS, 1);
    send__simple_command(WRDI);
    return nbytes;
}

int read_bytes(const uint16_t addr, uint8_t* buf, const uint8_t nbytes) {
    if (nbytes <= 0) { return 0; }

    //gpio_put(CS, 0);
    uint8_t cmd = READ;
    spi_write_blocking(SPI_BUS, &cmd, 1);
    uint8_t arr[2] = { addr/0x100, addr%0x100 };
    spi_write_blocking(SPI_BUS, arr, 2); //Code to cast uint16 address into two bytes to be sent through SPI
    spi_read_blocking(SPI_BUS, NULL, buf, nbytes);
    //gpio_put(CS, 1);
    return nbytes;
}

//Determine whether to do padding or have memory wrap around like circular array with some half_packets
int write_packet(uint8_t* buf) {
    address_write(cur_addr, buf, PACKET_SIZE);
    cur_addr += PACKET_SIZE;
    if (cur_addr > MAX_BYTES) { cur_addr = memory_start; }
    uint8_t arr[2] = {cur_addr / 0x100, cur_addr % 0x100}; // Code to cast uint16 address into two bytes
    address_write(memory_start - 2, arr, 2); //Update current address in memory
    return 1;
}

int read_packets(uint8_t* buf, int num_packets) {
    if (num_packets <= 0) { return 0; }

    uint16_t bytes_to_read = min(cur_addr - memory_start, num_packets * PACKET_SIZE);
    uint16_t bytes_read = num_packets * PACKET_SIZE - bytes_to_read;
    read_bytes(MAX_BYTES - bytes_read + 1, buf, bytes_read );
    read_bytes(cur_addr - bytes_to_read + 1, buf + bytes_read, bytes_to_read);
    return num_packets;
}
void mram_testing() {
    setup();

    while(true) {
        uint8_t my_buf[8] = {1, 2, 3, 4, 5, 6, 7, 8};
        address_write(1, my_buf, 1);
        uint8_t output[8];
        read_bytes(1, output, 1);

    
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
    }
}
