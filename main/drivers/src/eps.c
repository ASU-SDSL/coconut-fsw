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
//static const double CURRENT_LSB = 0.00001525879; // expected current 0.5A, (max expected current / 2^15)
//static const uint8_t CAL[] = {0x68, 0xDB}; // this is 26843, but needs to be input as an array of uint8_t
// 					// trunc(0.04096 / (Current_LSB * Rshunt)) Rshunt = 0.1 ohms?
// //static const uint8_t CAL[] = {0x14, 0x7A, 0x89}; //trunc(0.04096 / (CURRENT_LSB * SHUNT_RESISTOR)) can be this, too many bits
// static const double POWER_LSB = 0.00030517578; // 20 * Current_LSB
// static const float SHUNT_LSB = 0.00001;
// static const float BUS_LSB = 0.004;
// static const float SHUNT_RESISTOR = 0.1; // ohms

static const double CURRENT_LSB = 0.1; // expected current 0.5A, (max expected current / 2^15)
static const uint8_t CAL[] = {0x10, 0x00}; // this is 26843, but needs to be input as an array of uint8_t
					// trunc(0.04096 / (Current_LSB * Rshunt)) Rshunt = 0.1 ohms?
static const double POWER_LSB = 20 * CURRENT_LSB; //0.002; // 20 * Current_LSB
static const float SHUNT_LSB = 0.00001;
static const float BUS_LSB = 0.004;
static const int CURRENT_DEVIDER_MA = 10;
static const int POWER_MULTIPLIER_MW = 2;

//uint16_t config = (0x2000) | (0x1800) |  (0x0180) | (0x0018) | (0x07);
static const uint8_t CONFIG[] = {0x39,0x9f};

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

void print_each_register(i2c_inst_t* i2c){
	int reg[6] = {0x00, 0x01, 0x02, 0x03, 0x04, 0x05};

	uint8_t buf[2];
	for(int i = 0; i < 6; i++){
		i2c_read_from_register(i2c, INA219_ADDR, reg[i], buf, 2);
		printf("%02x", buf[0]);
		printf("%02x ", buf[1]);
		
	}
	printf("\n");
}

void print_all_registers(i2c_inst_t* i2c){

	uint8_t buf[12];
	i2c_read_from_register(i2c, INA219_ADDR, 0x00, buf, 2);

	for(int i = 0; i < 12; i++){
		printf("%x", buf[i]);
		if(i % 2 == 1) printf(" ");
	}
	printf("\n");
}

int calibrate(i2c_inst_t *i2c){

	// Program calibration register
	uint8_t* data = CAL;
	if(i2c_write_to_register(i2c, INA219_ADDR, REG_CALIB, data, 2) != 0){
		return 0;
	}

	// Test calibration register 
	i2c_read_from_register(i2c, INA219_ADDR, REG_CALIB, data, 2);

	printf("0x%x\r\n", data[0]);
	printf("0x%x\r\n", data[1]);

	return 1;

}

int config(i2c_inst_t *i2c){
	//Program config register 
	uint8_t* config = CONFIG;

	if(i2c_write_to_register(i2c, INA219_ADDR, REG_CONFIG, config, 2) != 0){
		return 0;
	}

	// Test calibration register
	i2c_read_from_register(i2c, INA219_ADDR, REG_CONFIG, config, 2);

	printf("0x%x\r\n", config[0]);
	printf("0x%x\r\n", config[1]);

	return 1;
}

int getVShunt(i2c_inst_t *i2c,
				const uint8_t addr,
				const uint8_t reg_vs,
				float *output_buf) {

	uint8_t buf;

	if (i2c_read_from_register(i2c, addr, reg_vs, &buf, 2) < 0) {
		return 0;
	}

	printf("raw vshunt: %d\n", buf);

	*output_buf = (buf >> 3)*(SHUNT_LSB);
	return 1;
}

int getVShuntNew(i2c_inst_t *i2c,
					const uint8_t addr,
					const uint8_t reg_vs,
					float *output_buf) {
	uint8_t buf[2];

	if(i2c_read_from_register(i2c, addr, reg_vs, buf, 2) < 0){
		return 0;
	}


	uint16_t bufComb = buf[1];
	bufComb = (bufComb << 8) | buf[2];

	printf("raw vshunt: %d\n", bufComb);

	*output_buf = bufComb * 0.01; // mV
	return 1;

}

int getVBus(i2c_inst_t *i2c,
			const uint8_t addr,
			const uint8_t reg_vb,
			float *output_buf) {

	uint8_t buf;

	if (i2c_read_from_register(i2c, addr, reg_vb, &buf, 2) < 0) {
		return 0;
	}

	printf("raw bus: %d\n", buf);

	*output_buf = (buf >> 3)*(BUS_LSB); // in volts
	return 1;
}

int getVBusNew(i2c_inst_t *i2c,
				const uint8_t addr,
				const uint8_t reg_vb,
				float *output_buf) {
	uint8_t buf[2];

	if (i2c_read_from_register(i2c, addr, reg_vb, buf, 2) < 0) {
		return 0;
	}

	uint16_t bufComb = buf[1];
	bufComb = (bufComb << 8) | buf[2];

	printf("raw bus: %x\n", bufComb);

	*output_buf = ((int16_t)(((bufComb >> 3) * 4))) * 0.001;
	return 1;
}

// power in Watts
int getPower(i2c_inst_t *i2c,
				const uint8_t addr,
				const uint8_t reg_p,
				double *output_buf) {

	uint8_t buf;

	// error codes are < 0
	if (i2c_read_from_register(i2c, addr, reg_p, &buf, 2) < 0) {
		return 0;
	}
	printf("raw power: %d\n", buf);
        
	*output_buf = (buf)*(POWER_LSB);
	return 1;
}

int getPowerNew(i2c_inst_t *i2c,
				const uint8_t addr,
				const uint8_t reg_p,
				double *output_buf) {

	uint8_t buf[2];

	calibrate(i2c);

	// error codes are < 0
	if (i2c_read_from_register(i2c, addr, reg_p, buf, 2) < 0) {
		return 0;
	}

	uint16_t bufComb = buf[1];
	bufComb = (bufComb << 8) | buf[2];

	printf("raw power: %d\n", bufComb);
        
	*output_buf = (bufComb)*(POWER_MULTIPLIER_MW);
	return 1;
}

// current in Amps
int getCurrent(i2c_inst_t *i2c,
				const uint8_t addr,
				const uint8_t reg_c,
				double *output_buf) {

	uint8_t buf;

	if (i2c_read_from_register(i2c, addr, reg_c, &buf, 2) < 0) {
		return 0;
	}
	printf("raw current: %d\n", buf);

	*output_buf = (buf)*(CURRENT_LSB);
	return 1;
}

int getCurrentNew(i2c_inst_t *i2c,
					const uint8_t addr,
					const uint8_t reg_c,
					double *output_buf) {

	uint8_t buf[2];

	calibrate(i2c);

	if (i2c_read_from_register(i2c, addr, reg_c, buf, 2) < 0) {
		return 0;
	}

	uint16_t bufComb = buf[1];
	bufComb = (bufComb << 8) | buf[2];

	printf("raw current: %d\n", bufComb);

	*output_buf = ((double)(bufComb)) / (CURRENT_DEVIDER_MA);
	return 1;
}

void eps_test() {
	printf("STARTING EPS TEST\n");

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
	//config_i2c0();

	// Program Calibration register
	// data = CAL[0]; // point to start of array
	// i2c_write_to_register(i2c, INA219_ADDR, REG_CALIB, &data, 2);

	// // Test: read Calibration register
	// i2c_read_from_register(i2c, INA219_ADDR, REG_CALIB, &data, 2);
	// printf("0x%02x\r\n", data);
	print_all_registers(i2c);

	printf("CALIBRATING\n");
	calibrate(i2c);

	print_all_registers(i2c);

	printf("CONFIGURING\n");
	config(i2c);

	print_all_registers(i2c);

	// Wait
	sleep_ms(2000);

	printf("STARTING TEST\n");

	// Loop 10 times
	for(int i = 0; i < 20; i++){
		print_all_registers(i2c);
		float shunt;
		float vbus;
		double power;
		double current;

		// // Read registers (16 bits each)
		// if (getVShunt(i2c, INA219_ADDR, REG_SHUNT, &shunt) == 0) {
		// 	printf("Shunt voltage error\n");
		// }
		// else {
		// 	printf("Shunt voltage: %.2f V\r\n", shunt);
		// }

		// if (getVBus(i2c, INA219_ADDR, REG_BUS, &vbus) == 0) {
		// 	printf("Bus voltage error\n");
		// }
		// else {
		// 	printf("Bus voltage: %.2f V\r\n", vbus);
		// }

		// if (getPower(i2c, INA219_ADDR, REG_POWER, &power) == 0) {
		// 	printf("Power error\n");
		// }
		// else {
		// 	printf("Power: %.2f W\r\n", power);
		// }
		
		// if (getCurrent(i2c, INA219_ADDR, REG_CURRENT, &current) == 0) {
		// 	printf("Current error\n");
		// }
		// else {
		// 	printf("Current: %.2f A\r\n", current);
		// }
		
		printf("\nTESTING NEW FUNCTIONS BASED OFF ARDUINO\n");
		// Read registers (16 bits each)
	
		if (getVShuntNew(i2c, INA219_ADDR, REG_SHUNT, &shunt) == 0) {
			printf("Shunt voltage error\n");
		}
		else {
			printf("Shunt voltage: %.2f mV\r\n", shunt);
		}

		
		if (getVBusNew(i2c, INA219_ADDR, REG_BUS, &vbus) == 0) {
			printf("Bus voltage error\n");
		}
		else {
			printf("Bus voltage: %.2f V\r\n", vbus);
		}

		
		if (getPowerNew(i2c, INA219_ADDR, REG_POWER, &power) == 0) {
			printf("Power error\n");
		}
		else {
			printf("Power: %.2f mW\r\n", power);
		}
		
		
		if (getCurrentNew(i2c, INA219_ADDR, REG_CURRENT, &current) == 0) {
			printf("Current error\n");
		}
		else {
			printf("Current: %.2f mA\r\n", current);
		}
		printf("\n");

		sleep_ms(2000);
	}
}
