#include "i2cDriver.h"


int i2c_driver_write(	i2c_inst_t *i2c,
				const uint8_t addr,
				const uint8_t reg,
				uint8_t *buf,
				const uint8_t nbytes){
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

int i2c_driver_read(	i2c_inst_t *i2c,
				const uint8_t addr,
				const uint8_t reg,
				uint8_t *output_buf,
				const uint8_t nbytes){
    i2c_init(i2c, 100 * 1000); // added from eps to mag 

	int num_bytes_read = 0;

	if (nbytes < 1) 
	{
        logln_info("too little bytes");
		return 0;
	}

    /**
     * in eps.c:
     * i2c_write_blocking(i2c, addr, &reg, 1, true);
     * num_bytes_read = i2c_read_blocking(i2c, addr, output_buf, nbytes, false);
    */

    logln_info("read");
	int ret = i2c_write_timeout_us(i2c, addr, &reg, 1, true, 1000000);
	num_bytes_read = i2c_read_timeout_us(i2c, addr, output_buf, nbytes, false, 1000000);

    logln_info("here: %d", ret);

	return num_bytes_read;
}

