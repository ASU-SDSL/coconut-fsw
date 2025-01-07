#include "i2c.h"

// Error codes
#define MAX17048_READ_ERROR -1
#define MAX17048_DEVICE_NOT_READY_ERROR -2
#define MAX17048_VALUE_READ_ERROR -3
#define MAX17048_RESET_ERROR -4
#define MAX17048_WRITE_ERROR -5


/*
* Turn on IC and bring out of sleep mode
*/
int max17048Wake(i2c_inst_t *i2c);

/*
* Read voltage of battery
*/
int max17048CellVoltage(i2c_inst_t *i2c, float *voltage_out);

/*
* Read voltage of battery
*/
int max17048CellVoltage(i2c_inst_t *i2c, float *voltage_out);

/*
* For use in a packet; takes up less bytes; conversion done on the ground
* Returns 2 uint8_ts
*/
int max17048CellVoltageRaw(i2c_inst_t *i2c, uint8_t *voltage_out);

/*
* Get percentage, raw version outputs 2 bytes
*/
int max17048CellPercentage(i2c_inst_t *i2c, float *percentage_out);
int max17048CellPercentageRaw(i2c_inst_t *i2c, uint8_t *percentage_out);
