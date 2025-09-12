#include "FreeRTOS.h"
#include "task.h"

#include "vega_ant.h"

#define VEGA_ANT_ADDR 0x55
#define STATUS_COMMAND 0x05
#define BURN_A_COMMAND 0x01
#define BURN_B_COMMAND 0x02 

#define VEGA_ANT_GPIO_PINA 6
#define VEGA_ANT_GPIO_PINB 15


uint8_t vega_ant_status(i2c_inst_t* i2c, uint8_t* output){
    if(!i2c_read_from_register(i2c, VEGA_ANT_ADDR, STATUS_COMMAND, output, 1)){
        return 1; 
    }
    return 0; 
}

uint8_t vega_ant_deploy(i2c_inst_t* i2c) {
    uint8_t buf = 0; // garbage because we aren't actually writing to a register 

    if(!i2c_write_to_register(i2c, VEGA_ANT_ADDR, BURN_A_COMMAND, &buf, 1)){
        return 1; 
    }

    if(!i2c_write_to_register(i2c, VEGA_ANT_ADDR, BURN_B_COMMAND, &buf, 1)){
        return 1; 
    }

    // Toggle both deploy GPIOs
    gpio_init(VEGA_ANT_GPIO_PINA);
    gpio_init(VEGA_ANT_GPIO_PINB);

    gpio_set_dir(VEGA_ANT_GPIO_PINA, GPIO_OUT);
    gpio_set_dir(VEGA_ANT_GPIO_PINB, GPIO_OUT);

    gpio_put(VEGA_ANT_GPIO_PINA, 1);
    gpio_put(VEGA_ANT_GPIO_PINB, 1);

    vTaskDelay(200/portTICK_PERIOD_MS);

    gpio_put(VEGA_ANT_GPIO_PINA, 0);
    gpio_put(VEGA_ANT_GPIO_PINB, 0);

    return 0; 
}
