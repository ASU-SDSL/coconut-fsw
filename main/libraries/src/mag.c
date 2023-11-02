#include "mag.h"

// Slave Address, depends on SDO/SA1 pin connection
static uint8_t SAD = 0x1E; 
//static uint8_t SAD = 0x1C;

// Registers
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

int reg_read(i2c_inst_t *i2c, const uint8_t addr, const uint8_t reg, uint8_t *output_buf, const uint8_t nbytes) 
{
	int num_bytes_read = 0;

	if (nbytes < 1) 
	{
		return 0;
	}

	i2c_write_blocking(i2c, addr, &reg, 1, true);
	num_bytes_read = i2c_read_blocking(i2c, addr, output_buf, nbytes, false);

	return num_bytes_read;
}

int reg_write(i2c_inst_t *i2c, const uint8_t addr, const uint8_t reg, uint8_t *buf, const uint8_t nbytes) 
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

