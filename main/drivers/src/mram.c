#include "mram.h"
#include "FreeRTOS.h"

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
    send_simple_command(WAKE);

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

int write_bytes(uint32_t addr, uint8_t* buf, const uint8_t nbytes) {
    if (nbytes <= 0) { return 0; }

    send_simple_command(WREN);

    uint32_t bendaddr = __builtin_bswap32(addr);
    uint8_t *p_bendaddr = &bendaddr;

    uint8_t arr[4] = {WRITE, p_bendaddr[0], p_bendaddr[1], p_bendaddr[2]};
   
    gpio_put(CS, 0);
    spi_write_blocking(SPI_BUS, arr, 4);
    spi_write_blocking(SPI_BUS, buf, nbytes);
    gpio_put(CS, 1);

    send_simple_command(WRDI);
    
    return nbytes;
}

int read_bytes(uint32_t addr, uint8_t* buf, const uint8_t nbytes) {
    if (nbytes <= 0) { return 0; }

    gpio_put(CS, 0);

    uint32_t bendaddr = __builtin_bswap32(addr);
    uint8_t *p_bendaddr = &bendaddr;

    uint8_t arr[4] = {READ, p_bendaddr[0], p_bendaddr[1], p_bendaddr[2]};

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
