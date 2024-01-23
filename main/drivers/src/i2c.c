#include "i2c.h"

void config_i2c0() {
   
    // Ports
    i2c_init(i2c, I2CSpeed);

	// Set SCL and SDA to I2C0 pins defined in i2c.h
    gpio_set_function(I2C0_SDA_GPIO, GPIO_FUNC_I2C);
    gpio_set_function(I2C0_SCL_GPIO, GPIO_FUNC_I2C);
    //bi_decl(bi_2pins_with_func(PICO_DEFAULT_I2C_SDA_PIN, PICO_DEFAULT_I2C_SCL_PIN, GPIO_FUNC_I2C)); I don't think this is needed, just provides more info in the binary

}

int i2c_write_to_register(	i2c_inst_t *i2c,
							const uint8_t addr,
							const uint8_t reg,
							uint8_t *buf,
							const uint8_t nbytes){
    
	i2c_write_blocking(i2c, RTC_ADDR, &reg, 1, false);
    i2c_read_blocking(i2c, RTC_ADDR, &temp_h, 1, false);

	if (nbytes < 1) {
		return 0;
	}

	int num_bytes_written;

	// create message, putting the register first (mes is just [reg, [buf]])
	uint8_t msg[nbytes + 1];
	msg[0] = reg;
	for (int i = 0; i < nbytes; i++) {
		msg[i + 1] = buf[i];
	}

	num_bytes_written = i2c_write_timeout_us(i2c, addr, msg, (nbytes + 1), false, I2CTimeout_us);

	return num_bytes_written;
}

int i2c_read_from_register(	i2c_inst_t *i2c,
							const uint8_t addr,
							const uint8_t reg,
							uint8_t *output_buf,
							const uint8_t nbytes){

	if (nbytes < 1) {
		return 0; // 0 bytes read
	}

	int ret = i2c_write_timeout_us(i2c, addr, &reg, 1, false, I2CTimeout_us);
	num_bytes_read = i2c_read_timeout_us(i2c, addr, output_buf, nbytes, false, I2CTimeout_us);

	return num_bytes_read;
}

