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
static const uint8_t TEMP_OUT_L = 0x2E;
static const uint8_t TEMP_OUT_H = 0x2F;
static const uint8_t INT_CFG = 0x30;
static const uint8_t INT_SRC = 0x31;
static const uint8_t INT_THIS_L = 0x32;
static const uint8_t INT_THIS_H = 0x33;

// Not sure what this was for, should not be needed
//uint i2c_init (i2c_inst_t *i2c, uint 100 * 1000) //initialization of i2c

int reg_read_m(i2c_inst_t *i2c, const uint8_t addr, const uint8_t reg, uint8_t *output_buf, const uint8_t nbytes) 
{
    i2c_init(i2c, 100 * 1000);

	int num_bytes_read = 0;

	if (nbytes < 1) 
	{
        logln_info("too little bytes");
		return 0;
	}

    logln_info("read");
	int ret = i2c_write_timeout_us(i2c, addr, &reg, 1, true, 1000000);
	num_bytes_read = i2c_read_timeout_us(i2c, addr, output_buf, nbytes, false, 1000000);

    logln_info("here: %d", ret);

	return num_bytes_read;
}

// unused as far as I can tell
int reg_write_m(i2c_inst_t *i2c, const uint8_t addr, const uint8_t reg, uint8_t *buf, const uint8_t nbytes) 
{
	if (nbytes < 1) 
	{
		return 0;
	}

	int num_bytes_written;
	uint8_t msg[nbytes + 1];

	msg[0] = reg;
	for (int i = 0; i < nbytes; i++) {
		msg[i + 1] = buf[i];
	}

	num_bytes_written = i2c_write_blocking(i2c, addr, msg, (nbytes + 1), false);

	return num_bytes_written;
}

int config_mag(i2c_inst_t *i2c){
    uint8_t buf; 
    i2c_read_from_register(i2c, SAD, CTRL_REG1, &buf, 1);
    printf("ctrl 1 raw: %02x\n", buf);

    buf = buf | 0b10100000;

    printf("new ctrl 1 raw: %02x\n", buf);

    int success = i2c_write_to_register(i2c, SAD, CTRL_REG1, &buf, 1);

    i2c_read_from_register(i2c, SAD, CTRL_REG2, &buf, 1);
    printf("crtl 2: raw: %02x\n", buf);
    
    return success;
}

int get_x_output(i2c_inst_t *i2c) { //defines function

    uint8_t buf_low; //buf means buffer, allocates space for data to be entered in an 8 bit number (uint8_t)
    i2c_read_from_register(i2c, SAD, OUT_X_L, &buf_low, 1); // taken from eps library, 0x28 is the location

    uint8_t* buf_high;
    i2c_read_from_register(i2c, SAD, OUT_X_H, buf_high, 1);

    //high =   00000010
    //low =    00000001
    //high is just the higher register, probably just the higher value

    int x_out = (int) buf_low | ((int) buf_high << 8); // or ?logic operator, basically makes 0 or 1 = 1
    //0 | 1 = 1
    //01 | 10 = 11
    // 0 | 0 = 0

    //<< 8 pushes values 8 to the left
    //0000000 00000000 00000000 00000001
    //0000000 00000000 00000010 <<<<<<<<
    //or operator
    //0000000 00000000 00000010 00000001
    return x_out;
}

uint16_t get_x_output_raw(i2c_inst_t *i2c) { 

    uint8_t buf_low; //buf means buffer, allocates space for data to be entered in an 8 bit number (uint8_t)
    i2c_read_from_register(i2c, SAD, OUT_X_L, &buf_low, 1); 

    uint8_t buf_high;
    i2c_read_from_register(i2c, SAD, OUT_X_H, &buf_high, 1);

    //high =   00000010
    //low =    00000001
    //high is just the higher register, probably just the higher value

    uint16_t x_out = (uint16_t) buf_low | ((uint16_t) buf_high << 8); // or ?logic operator, basically makes 0 or 1 = 1
    //0 | 1 = 1
    //01 | 10 = 11
    // 0 | 0 = 0

    //<< 8 pushes values 8 to the left
    //0000000 00000000 00000000 00000001
    //0000000 00000000 00000010 <<<<<<<<
    //or operator
    //0000000 00000000 00000010 00000001
    return x_out;
}

int get_y_output(i2c_inst_t *i2c) { //Y output

    uint8_t buf_low;
    i2c_read_from_register(i2c, SAD, OUT_Y_L, &buf_low, 1);

    uint8_t buf_high;
    i2c_read_from_register(i2c, SAD, OUT_Y_H, &buf_high, 1);

    int y_out = (int) buf_low | ((int) buf_high << 8);
    return y_out;
}

uint16_t get_y_output_raw(i2c_inst_t *i2c) { //Y output

    uint8_t buf_low;
    i2c_read_from_register(i2c, SAD, OUT_Y_L, &buf_low, 1);

    uint8_t buf_high;
    i2c_read_from_register(i2c, SAD, OUT_Y_H, &buf_high, 1);

    uint16_t y_out = ((uint16_t) buf_low) | (((uint16_t) buf_high) << 8);
    return y_out;
}

int get_z_output(i2c_inst_t *i2c){ //Z output

    uint8_t buf_low;
    i2c_read_from_register(i2c, SAD, OUT_Z_L, &buf_low, 1);

    uint8_t buf_high;
    i2c_read_from_register(i2c, SAD, OUT_Z_H, &buf_high, 1);

    int z_out = (int) buf_low | ((int) buf_high << 8);
    return z_out;

}

uint16_t get_z_output_raw(i2c_inst_t *i2c){ //Z output

    uint8_t buf_low;
    i2c_read_from_register(i2c, SAD, OUT_Z_L, &buf_low, 1);

    uint8_t buf_high;
    i2c_read_from_register(i2c, SAD, OUT_Z_H, &buf_high, 1);

    uint16_t z_out = (uint16_t) buf_low | ((uint16_t) buf_high << 8);
    return z_out;

}

int get_temp_output(i2c_inst_t *i2c){ //Temperature output

    uint8_t buf_low;
    i2c_read_from_register(i2c, SAD, TEMP_OUT_L, &buf_low, 1);

    uint8_t buf_high;
    i2c_read_from_register(i2c, SAD, TEMP_OUT_H, &buf_high, 1);

    int temp_out = (int) buf_low | ((int) buf_high << 8);
    return temp_out;

}

uint16_t get_temp_output_raw(i2c_inst_t *i2c){ //Temperature output

    uint8_t buf_low;
    i2c_read_from_register(i2c, SAD, TEMP_OUT_L, &buf_low, 1);

    uint8_t buf_high;
    i2c_read_from_register(i2c, SAD, TEMP_OUT_H, &buf_high, 1);

    uint16_t temp_out = (uint16_t) buf_low | ((uint16_t) buf_high << 8);
    return temp_out;

}

uint8_t get_mag_status(i2c_inst_t *i2c) { //Indicates if data is available/overrun

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
    config_mag(i2c);

    // Wait
    sleep_ms(2000);

    // Loop 1000 times
    for(int i = 0; i < 10; i++){

        printf("Status: %d\n", get_mag_status(i2c));
        printf("Status (raw): %02x\n", get_mag_status(i2c));

        printf("X output: %d\n", get_x_output(i2c));
        printf("X output (raw): %04x\n", get_x_output_raw(i2c));

        printf("Y output: %d\n", get_y_output(i2c));
        printf("Y output (raw): %04x\n", get_y_output_raw(i2c));

        printf("Z output: %d\n", get_z_output(i2c));
        printf("Z output (raw): %04x\n", get_z_output(i2c));

        printf("Get Temp Output: %d\n", get_temp_output(i2c));
        printf("Get Temp Output (raw): %04x\n", get_temp_output_raw(i2c));

        sleep_ms(500);

    }

    

}