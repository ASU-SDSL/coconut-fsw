#include "EpsSensor.h"

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
static const unsigned int CAL = 26843;
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

	return num_bytes_written;
}

int reg_read(	i2c_inst_t *i2c,
		const uint8_t addr,
		const uint8_t reg,
		uint8_t *buf,
		const uint8_t nbytes) {

	int num_bytes_read = 0;

	if (nbytes < 1) {
		return 0;
	}

	i2c_write_blocking(i2c, addr, &reg, 1, true);
	num_bytes_read = i2c_read_blocking(i2c, addr, buf, nbytes, false);

	return num_bytes_read;
}

float getVShunt(        i2c_inst_t *i2c,
                        const uint8_t addr,
                        const uint8_t reg_vs,
                        uint16_t *buf) {

        float vshunt;

        reg_read(i2c, addr, reg_vs, buf, 2);
        vshunt = (buf >> 3)*(SHUNT_LSB);
        return vshunt;
}

float getVBus(          i2c_inst_t *i2c,
                        const uint8_t addr,
                        const uint8_t reg_vb,
                        uint16_t *buf) {

        float vbus;

        reg_read(i2c, addr, reg_vb, buf, 2);
        vbus = (buf >> 3)*(BUS_LSB);
        return vbus;
}

double getPower(	i2c_inst_t *i2c,
          	 	const uint8_t addr,
	                const uint8_t reg_p,
         	        uint16_t *buf) {

	double power;

	reg_read(i2c, addr, reg_p, buf, 2);
        power = (buf)*(POWER_LSB);
	return power;
}

double getCurrent(      i2c_inst_t *i2c,
                        const uint8_t addr,
                        const uint8_t reg_c,
                        uint8_t *buf) {

        double current;

        reg_read(i2c, addr, reg_c, buf, 2);
        current = (buf)*(CURRENT_LSB);
        return current;
}

int main() {

	// Pins
	const uint sda_pin = 6;
	const uint scl_pin = 7;

	// Ports
	i2c_inst_t *i2c = i2c0;

	// Buffer to store raw reads
	uint16_t data;

	// Floats for measurements
	float v_shunt;
	float v_bus;
	double power;
	double current;

	// Initialize chosen serial port
	stdio_init_all();

	// Initialize I2C port at 400 kHz
	i2c_init(i2c, 400 * 1000);

	// Initialize I2C pins
	gpio_set_function(sda_pin, GPIO_FUNC_I2C);
	gpio_set_function(scl_pin, GPIO_FUNC_I2C);

	// Program Calibration register
	data = CAL;
	reg_write(i2c, INA219_ADDR, REG_CALIB, &data, 2);

	// Test: read Calibration register
	reg_read(i2c, INA219_ADDR, REG_CALIB, data, 2);
	printf("0x%02x\r\n", data);

	// Wait
	sleep_ms(2000);

	// Loop
	while (true) {

		// Read registers (16 bits each)
		v_shunt = getVShunt(i2c, INA219_ADDR, REG_SHUNT, data);
		printf("Shunt voltage: %.2f V\r\n", v_shunt);

		v_bus = getVBus(i2c, INA219_ADDR, REG_BUS, data);
                printf("Bus voltage: %.2f V\r\n", v_bus);

		power = getPower(i2c, INA219_ADDR, REG_POWER, data);
                printf("Power: %.2d W\r\n", power);

		current = getCurrent(i2c, INA219_ADDR, REG_CURRENT, data);
                printf("Current: %.2d A\r\n", current);

		sleep_ms(100);
	}
}
