#include "eps.h"

// I2C address
static const uint8_t INA219_ADDR = 0x40;

// Registers
static const uint8_t REG_CONFIG = 0x00;
static const uint8_t REG_SHUNT = 0x01;
static const uint8_t REG_BUS = 0x02;
static const uint8_t REG_POWER = 0x03;
static const uint8_t REG_CURRENT = 0x04;
static const uint8_t REG_CALIB = 0x05;

// Other constants
static const double CURRENT_LSB = 0.00001525879;
static const uint8_t CAL[] = {0x68, 0xDB}; // this is 26843, but needs to be input as an array of uint8_t
static const double POWER_LSB = 0.00030517578;
static const float SHUNT_LSB = 0.00001;
static const float BUS_LSB = 0.004;

int reg_write(	i2c_inst_t *i2c,
				const uint8_t addr,
				const uint8_t reg,
				uint8_t *buf,
				const uint8_t nbytes);

int reg_read(	i2c_inst_t *i2c,
				const uint8_t addr,
				const uint8_t reg,
				uint8_t *buf,
				const uint8_t nbytes);

int reg_write(	i2c_inst_t *i2c,
                const uint8_t addr,
                const uint8_t reg,
                uint8_t *buf,
                const uint8_t nbytes) {
	
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

int reg_read(	i2c_inst_t *i2c,
				const uint8_t addr,
				const uint8_t reg,
				uint8_t *output_buf,
				const uint8_t nbytes) {

	int num_bytes_read = 0;

	if (nbytes < 1) {
		return 0;
	}

	i2c_write_blocking(i2c, addr, &reg, 1, true);
	num_bytes_read = i2c_read_blocking(i2c, addr, output_buf, nbytes, false);

	return num_bytes_read;
}

int getVShunt(        i2c_inst_t *i2c,
                        const uint8_t addr,
                        const uint8_t reg_vs,
                        float *output_buf) {

		uint8_t buf;

		if (i2c_read_from_register(i2c, addr, reg_vs, &buf, 2) < 0) {
			return 0;
		}

		*output_buf = (buf >> 3)*(SHUNT_LSB);
        return 1;
}

int getVBus(          i2c_inst_t *i2c,
                        const uint8_t addr,
                        const uint8_t reg_vb,
                        float *output_buf) {

        uint8_t buf;

        if (i2c_read_from_register(i2c, addr, reg_vb, &buf, 2) < 0) {
			return 0;
		}

        *output_buf = (buf >> 3)*(BUS_LSB);
        return 1;
}

int getPower(	i2c_inst_t *i2c,
          	 		const uint8_t addr,
	                const uint8_t reg_p,
         	        double *output_buf) {

	uint8_t buf;

	// error codes are < 0
	if (i2c_read_from_register(i2c, addr, reg_p, &buf, 2) < 0) {
		return 0;
	}
        
	*output_buf = (buf)*(POWER_LSB);
	return 1;
}

int getCurrent(      i2c_inst_t *i2c,
                        const uint8_t addr,
                        const uint8_t reg_c,
                        double *output_buf) {

        uint8_t buf;

        if (i2c_read_from_register(i2c, addr, reg_c, &buf, 2) < 0) {
			return 0;
		}

        *output_buf = (buf)*(CURRENT_LSB);
        return 1;
}

int eps_test() {

	// Ports
	i2c_inst_t *i2c = i2c0;

	// Buffer to store raw reads
	uint8_t data;
	float buf;

	// Floats for measurements
	float v_shunt;
	float v_bus;
	double power;
	double current;

	// Setup i2c
	config_i2c0();

	// Program Calibration register
	data = CAL[0]; // point to start of array
	i2c_write_to_register(i2c, INA219_ADDR, REG_CALIB, &data, 2);

	// Test: read Calibration register
	i2c_read_from_register(i2c, INA219_ADDR, REG_CALIB, &data, 2);
	printf("0x%02x\r\n", data);

	// Wait
	sleep_ms(2000);

	// Loop 1000 times
	for(int i = 0; i < 1000; i++){

		// Read registers (16 bits each)
		float shunt;
		if (getVShunt(i2c, INA219_ADDR, REG_SHUNT, &shunt) == 0) {
			printf("Shunt voltage error\n");
		}
		else {
			printf("Shunt voltage: %.2f V\r\n", shunt);
		}

		float vbus;
		if (getVBus(i2c, INA219_ADDR, REG_BUS, &vbus) == 0) {
			printf("Bus voltage error\n");
		}
		else {
			printf("Bus voltage: %.2f V\r\n", vbus);
		}

		double power;
		if (getPower(i2c, INA219_ADDR, REG_POWER, &power) == 0) {
			printf("Power error\n");
		}
		else {
			printf("Power: %.2f W\r\n", power);
		}
		
		double current;
		if (getCurrent(i2c, INA219_ADDR, REG_CURRENT, &current) == 0) {
			printf("Current error\n");
		}
		else {
			printf("Current: %.2f A\r\n", current);
		}

		sleep_ms(100);
	}
}
