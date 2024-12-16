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

int calibrate(i2c_inst_t *i2c){

	// Program calibration register
	uint8_t* data = (uint8_t*)CAL;
	if(i2c_write_to_register(i2c, INA219_ADDR, REG_CALIB, data, 2)){
		return 1;
	}

	// Test calibration register 
	//i2c_read_from_register(i2c, INA219_ADDR, REG_CALIB, data, 2);

	//printf("0x%x\r\n", data[0]);
	//printf("0x%x\r\n", data[1]);

	return 0;

}

int config(i2c_inst_t *i2c){
	//Program config register 
	uint8_t* config = (uint8_t*)CONFIG;

	if(i2c_write_to_register(i2c, INA219_ADDR, REG_CONFIG, config, 2)){
		return 1;
	}

	// Test calibration register
	//i2c_read_from_register(i2c, INA219_ADDR, REG_CONFIG, config, 2);

	//printf("0x%x\r\n", config[0]);
	//printf("0x%x\r\n", config[1]);

	return 0;
}



int getVShunt(i2c_inst_t *i2c,
				const uint8_t addr,
				const uint8_t reg_vs,
				float *output_buf) {
	uint8_t buf[2];

	if(i2c_read_from_register(i2c, addr, reg_vs, buf, 2)){
		return 1;
	}


	uint16_t bufComb = buf[0];
	bufComb = (bufComb << 8) | buf[1];

	// printf("raw vshunt: %d\n", bufComb);

	*output_buf = bufComb * 0.01; // mV
	return 0;

}

int getVShunt_raw(i2c_inst_t *i2c,
				const uint8_t addr,
				uint16_t *output_buf) {
	uint8_t buf[2];

	if(i2c_read_from_register(i2c, addr, REG_SHUNT, buf, 2)){
		return 1;
	}

	uint16_t bufComb = buf[0]; 
	bufComb = (bufComb << 8) | buf[1];

	// printf("raw vshunt: %d\n", bufComb);

	*output_buf = bufComb; // * 0.01 = mV
	return 0;

}

int getVBus(i2c_inst_t *i2c,
				const uint8_t addr,
				const uint8_t reg_vb,
				float *output_buf) {
	uint8_t buf[2];

	if (i2c_read_from_register(i2c, addr, reg_vb, buf, 2)) {
		return 1;
	}

	uint16_t bufComb = buf[0];
	bufComb = (bufComb << 8) | buf[1];

	// printf("raw bus: %x\n", bufComb);

	*output_buf = ((int16_t)(((bufComb >> 3) * 4))) * 0.001; // volts
	return 0;
}

int getVBus_raw(i2c_inst_t *i2c,
				const uint8_t addr,
				uint16_t *output_buf) {
	uint8_t buf[2];

	if (i2c_read_from_register(i2c, addr, REG_BUS, buf, 2)) {
		return 1;
	}

	uint16_t bufComb = buf[0];
	bufComb = (bufComb << 8) | buf[1];

	// printf("raw bus: %x\n", bufComb);

	*output_buf = bufComb; // ((int16_t)(((bufComb >> 3) * 4))) * 0.001; // volts
	return 0;
}

int getPower(i2c_inst_t *i2c,
				const uint8_t addr,
				const uint8_t reg_p,
				double *output_buf) {

	uint8_t buf[2];

	calibrate(i2c);

	// error codes are < 0
	if (i2c_read_from_register(i2c, addr, reg_p, buf, 2)) {
		return 1;
	}

	uint16_t bufComb = buf[0];
	bufComb = (bufComb << 8) | buf[1];

	// printf("raw power: %d\n", bufComb);
        
	*output_buf = (bufComb)*(POWER_MULTIPLIER_MW);
	return 0;
}

int getPower_raw(i2c_inst_t *i2c,
				const uint8_t addr,
				uint16_t *output_buf) {

	uint8_t buf[2];

	calibrate(i2c);

	// error codes are < 0
	if (i2c_read_from_register(i2c, addr, REG_POWER, buf, 2)) {
		return 1;
	}

	uint16_t bufComb = buf[0];
	bufComb = (bufComb << 8) | buf[1];

	// printf("raw power: %d\n", bufComb);
        
	*output_buf = bufComb; // (bufComb)*(POWER_MULTIPLIER_MW); // mW
	return 0;
}

int getCurrent(i2c_inst_t *i2c,
					const uint8_t addr,
					const uint8_t reg_c,
					double *output_buf) {

	uint8_t buf[2];

	calibrate(i2c);

	if (i2c_read_from_register(i2c, addr, reg_c, buf, 2)) {
		return 1;
	}

	uint16_t bufComb = buf[0];
	bufComb = (bufComb << 8) | buf[1];

	// printf("raw current: %d\n", bufComb);

	*output_buf = ((double)(bufComb)) / (CURRENT_DEVIDER_MA);
	return 0;
}

int getCurrent_raw(i2c_inst_t *i2c,
					const uint8_t addr,
					uint16_t *output_buf) {

	uint8_t buf[2];

	calibrate(i2c);

	if (i2c_read_from_register(i2c, addr, REG_CURRENT, buf, 2)) {
		return 1;
	}

	uint16_t bufComb = buf[0];
	bufComb = (bufComb << 8) | buf[1];

	// printf("raw current: %d\n", bufComb);

	*output_buf = bufComb; // ((double)(bufComb)) / (CURRENT_DEVIDER_MA); // mA
	return 0;
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

	printf("CALIBRATING\n");
	calibrate(i2c);


	printf("CONFIGURING\n");
	config(i2c);


	printf("STARTING TEST\n");

	// Loop 10 times
	// for(int i = 0; i < 20; i++){
	// 	float shunt;
	// 	float vbus;
	// 	double power;
	// 	double current;

	// 	uint16_t raw; 

	// 	// // Read registers (16 bits each)
	// 	printf("\nRAW VALUES\n");
	// 	if (getVShunt_raw(i2c, &raw)) {
	// 		printf("Shunt voltage error\n");
	// 	}
	// 	else {
	// 		printf("Shunt voltage (raw): %04x \r\n", raw);
	// 	}

	// 	if (getVBus_raw(i2c, &raw)) {
	// 		printf("Bus voltage error\n");
	// 	}
	// 	else {
	// 		printf("Bus voltage (raw): %04x \r\n", raw);
	// 	}

	// 	if (getPower_raw(i2c, &raw)) {
	// 		printf("Power error\n");
	// 	}
	// 	else {
	// 		printf("Power (raw): %04x \r\n", raw);
	// 	}
		
	// 	if (getCurrent_raw(i2c, &raw)) {
	// 		printf("Current error\n");
	// 	}
	// 	else {
	// 		printf("Current: %04x \r\n", raw);
	// 	}
		
	// 	printf("PROCESSED VALUES\n");
	// 	// Read registers (16 bits each)
	
	// 	if (getVShunt(i2c, INA219_ADDR, REG_SHUNT, &shunt)) {
	// 		printf("Shunt voltage error\n");
	// 	}
	// 	else {
	// 		printf("Shunt voltage: %.2f mV\r\n", shunt);
	// 	}
	// 	// sleep_ms(5);
		
	// 	if (getVBus(i2c, INA219_ADDR, REG_BUS, &vbus)) {
	// 		printf("Bus voltage error\n");
	// 	}
	// 	else {
	// 		printf("Bus voltage: %.2f V\r\n", vbus);
	// 	}

	// 	// sleep_ms(5);

	// 	if (getCurrent(i2c, INA219_ADDR, REG_CURRENT, &current)) {
	// 		printf("Current error\n");
	// 	}
	// 	else {
	// 		printf("Current: %.2f mA\r\n", current);
	// 	}

	// 	if (getPower(i2c, INA219_ADDR, REG_POWER, &power)) {
	// 		printf("Power error\n");
	// 	}
	// 	else {
	// 		printf("Power: %.2f mW\r\n", power);
	// 	}
		
		// sleep_ms(5);

		// sleep_ms(2000);
	// }
}
