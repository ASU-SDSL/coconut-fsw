#include "mag.h"

//Slave Address
//static uint8_t SAD = 0x1E; Use if SDO/SA1 is connected to high
static uint8_t SAD = 0x1C;

//Registers
static const uint8_t WHO_AM_I = 0x0F;
static const uint8_t CTRL_REG1 = 0x20;
static const uint8_t CTRL_REG2 = 0x21;
static const uint8_t CTRL_REG3 = 0x22;
static const uint8_t CTRL_REG4 = 0x23;
static const uint8_t CTRL_REG5 = 0x24;
static const uint8_t STATUS_REG = 0x27;
static const uint8_t OUT_X_L = 0x28;
static const uint8_t OUT_X_H = 0x29;
static const uint8_t OUT_Y_L = 0x2A;
static const uint8_t OUT_Y_H = 0x2B;
static const uint8_t OUT_Z_L = 0x2C;
static const uint8_t OUT_Z_H = 0x2D;
static const uint8_t MAG_TEMP_OUT_L = 0x2E;
static const uint8_t MAG_TEMP_OUT_H = 0x2F;
static const uint8_t INT_CFG = 0x30;
static const uint8_t INT_SRC = 0x31;
static const uint8_t INT_THIS_L = 0x32;
static const uint8_t INT_THIS_H = 0x33;

/**
 * config constants, use reference: https://github.com/adafruit/Adafruit_LIS3MDL/
 * scale (for range of 4 gauss (default)) - divide xyz outputs by the scale to get 
 * readings in gauss
*/
int SCALE = 6842; 

// Not sure what this was for, should not be needed
//uint i2c_init (i2c_inst_t *i2c, uint 100 * 1000) //initialization of i2c

int mag_config(i2c_inst_t *i2c){
    int success = 0;
    // set performance mode
    // xy - high performance mode (-10- ----)
    // also enable temp sensor (1--- ----)
    // also set 155 Hz data rate (---- --1-)
    // combined (110- --1-)
    uint8_t buf;
    success += i2c_read_from_register(i2c, SAD, CTRL_REG1, &buf, 1);
    buf = (buf | 0b11000010) & 0b11011111;
    // printf("writing to CTRL_REG1: %02x\n", buf);
    success += i2c_write_to_register(i2c, SAD, CTRL_REG1, &buf, 1);
    i2c_read_from_register(i2c, SAD, CTRL_REG1, &buf, 1);
    // printf("%x\n", buf);

    // z - high performance mode (---- 10--)
    success += i2c_read_from_register(i2c, SAD, CTRL_REG4, &buf, 1);
    buf = (buf | 0b00001000) & 0b11111011;
    // printf("writing to CTRL_REG4: %02x\n", buf);
    success += i2c_write_to_register(i2c, SAD, CTRL_REG4, &buf, 1);

    // set range (this is default - currently no changes)

    // set operation mode
    success += i2c_read_from_register(i2c, SAD, CTRL_REG3, &buf, 1);
    buf = (buf & 0b11111100);
    // printf("writing to CTRL_REG3: %02x\n", buf);
    success += i2c_write_to_register(i2c, SAD, CTRL_REG3, &buf, 1);

    return success;
}

uint8_t mag_get_x(i2c_inst_t *i2c, int16_t* output) { //defines function

    uint8_t buf_low; //buf means buffer, allocates space for data to be entered in an 8 bit number (uint8_t)
    if(i2c_read_from_register(i2c, SAD, OUT_X_L, &buf_low, 1)){
        return 1; 
    }

    uint8_t buf_high;
    if(i2c_read_from_register(i2c, SAD, OUT_X_H, &buf_high, 1)){
        return 1; 
    }

    int16_t x_out = 0;// (int) buf_low | ((int) buf_high << 8); // or ?logic operator, basically makes 0 or 1 = 1
    x_out = ((x_out | buf_high) << 8) | buf_low;

    *output = x_out; 

    return 0;
}

uint8_t mag_get_y(i2c_inst_t *i2c, int16_t* output) { //Y output

    uint8_t buf_low;
    if(i2c_read_from_register(i2c, SAD, OUT_Y_L, &buf_low, 1)){
        return 1;
    }

    uint8_t buf_high;
    if(i2c_read_from_register(i2c, SAD, OUT_Y_H, &buf_high, 1)){
        return 1; 
    }

    int16_t y_out = 0; //(int) buf_low | ((int) buf_high << 8);
    y_out = ((y_out | buf_high) << 8) | buf_low;

    *output = y_out;
    return 0; 
}

uint8_t mag_get_z(i2c_inst_t *i2c, int16_t* output){ //Z output

    uint8_t buf_low;
    if(i2c_read_from_register(i2c, SAD, OUT_Z_L, &buf_low, 1)){
        return 1;
    }

    uint8_t buf_high;
    if(i2c_read_from_register(i2c, SAD, OUT_Z_H, &buf_high, 1)){
        return 1; 
    }

    int16_t z_out = 0; //(int) buf_low | ((int) buf_high << 8);
    z_out = ((z_out | buf_high) << 8) | buf_low;
    *output = z_out;
    return 0; 
}

uint8_t mag_get_temp(i2c_inst_t *i2c, int16_t* output){ //Temperature output

    uint8_t buf_low;
    if(i2c_read_from_register(i2c, SAD, MAG_TEMP_OUT_L, &buf_low, 1)){
        return 1; 
    }

    uint8_t buf_high;
    if(i2c_read_from_register(i2c, SAD, MAG_TEMP_OUT_H, &buf_high, 1)){
        return 1; 
    }

    int16_t temp_out = 0; //(int) buf_low | ((int) buf_high << 8);
    temp_out = ((temp_out | buf_high) << 8) | buf_low;
    *output = temp_out;
    return 0; 
}

uint8_t mag_get_status(i2c_inst_t *i2c) { //Indicates if data is available/overrun

	uint8_t buf;
	i2c_read_from_register(i2c, SAD, STATUS_REG, &buf, 1);

	// int status = (int) buf;
	// return status;
    return buf;
}

int mag_test(){

    i2c_inst_t *i2c = i2c0;

    // Setup i2c
    config_i2c0();

    // config mag
    mag_config(i2c);

    // Wait
    // sleep_ms(2000);

    // Loop 1000 times
    for(int i = 0; i < 100; i++){

        int16_t magbuf; 
        // printf("Status: %d\n", get_mag_status(i2c));
        // printf("Status (raw): %02x\n", get_mag_status(i2c));

        if(!mag_get_x(i2c, &magbuf)) {
            printf("X output: %d\n", magbuf);
            printf("X output (gauss): %f\n", (float)magbuf / SCALE);
        }
        else {
            printf("Error reading x\n");
        }

        if(!mag_get_y(i2c, &magbuf)) {
            printf("Y output: %d\n", magbuf);
            printf("Y output (gauss): %f\n", (float)magbuf / SCALE);
        }
        else {
            printf("Error reading y\n");
        }

        if(!mag_get_z(i2c, &magbuf)){ 
            printf("Z output: %d\n", magbuf);
            printf("Z output (gauss): %f\n", (float)magbuf / SCALE);
        }
        else {
            printf("Error reading z\n"); 
        }

        if(!mag_get_temp(i2c, &magbuf)){
            printf("Get Temp Output: %d\n", magbuf);
            printf("Get Temp Output Translated: %f\n", (float)magbuf / 256 + 25);
            printf("%x\n", magbuf);
        }
        else {
            printf("Error reading mag temp\n"); 
        }

        // sleep_ms(500);

    }

}