#include "FreeRTOS.h"
#include "task.h"

#include "vega_ant.h"
#include "log.h"

#define VEGA_ANT_ADDR 0x55
#define STATUS_COMMAND 0x05
#define BURN_A_COMMAND 0x01
#define BURN_B_COMMAND 0x02 
#define VEGA_ANT_GPIO_PIN 20 // TODO: update with real pin number

uint8_t vega_ant_status(i2c_inst_t* i2c, uint8_t* output){
    if(!i2c_read_from_register(i2c, VEGA_ANT_ADDR, STATUS_COMMAND, output, 1)){
        return 1; 
    }
    return 0; 
}

static uint8_t vega_ant_deploy_A(i2c_inst_t* i2c){
    uint8_t msg = BURN_A_COMMAND;

    return i2c_write_timeout_us(i2c, VEGA_ANT_ADDR, &msg, 1, false,
                         I2CTimeout_us);
}

static int vega_ant_deploy_B(i2c_inst_t* i2c){
    uint8_t msg = BURN_B_COMMAND;

    return i2c_write_timeout_us(i2c, VEGA_ANT_ADDR, &msg, 1, false,
                         I2CTimeout_us);
}

uint8_t vega_ant_deploy(i2c_inst_t* i2c){
    uint8_t buf = 0; // garbage because we aren't actually writing to a register 

    // burn A
    int res = vega_ant_deploy_A(i2c); 

    // check output 
    if(res == PICO_ERROR_GENERIC || res == PICO_ERROR_TIMEOUT){
        logln_error("error on burn A: %d", res); 
        return res; 
    }

    // delay for the state to go back to idle? ~10s 
    vTaskDelay(pdMS_TO_TICKS(10000)); 

    // burn B 
    res = vega_ant_deploy_B(i2c); 
    
    // check output 
    if(res == PICO_ERROR_GENERIC || res == PICO_ERROR_TIMEOUT){
        logln_error("error on burn B: %d", res); 
        return res; 
    }


    gpio_init(VEGA_ANT_GPIO_PIN);
    gpio_set_dir(VEGA_ANT_GPIO_PIN, GPIO_OUT);
    gpio_put(VEGA_ANT_GPIO_PIN, 1);
    vTaskDelay(200/portTICK_PERIOD_MS);
    gpio_put(VEGA_ANT_GPIO_PIN, 0);

    return 0; 
}
