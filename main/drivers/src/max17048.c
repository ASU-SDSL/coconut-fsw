#include "max17048.h"
#include "log.h"

#define MAX17048_I2C_ADDR 0x36

#define REG_VCELL 0x02
#define REG_SOC 0x04
#define REG_MODE 0x06
#define REG_VERSION 0x08
#define REG_HIBRT 0x0A
#define REG_CONFIG 0x0C
#define REG_STATUS 0x1A
#define REG_CMD 0xFE

#define MAX1704X_ALERTFLAG_RESET_INDICATOR 0x01 // Alert flag for IC reset

// Referenced from Adafruit_MAX1704X library - https://github.com/adafruit/Adafruit_MAX1704X/blob/main/

int max17048Wake(i2c_inst_t *i2c) {

    // Send reset signal, this will return an error code because the device will not ack
    uint8_t command[] = {0x54};
    i2c_write_to_register(i2c, MAX17048_I2C_ADDR, REG_CMD, command, 1);

    // Loop and wait until we can clear the flag to make sure it is on
    for (uint8_t retries = 0; retries < 3; retries++) {
        // Read status register, & with MAX1704X_ALERTFLAG_RESET_INDICATOR and try to write it back
        uint8_t buf[] = {0x00};
        int status = i2c_read_from_register(i2c, MAX17048_I2C_ADDR, REG_STATUS, buf, 1);
        if (status != 0) { continue; }
        buf[0] &= ~MAX1704X_ALERTFLAG_RESET_INDICATOR;
        status = i2c_write_to_register(i2c, MAX17048_I2C_ADDR, REG_STATUS, buf, 1);

        if (status == 0) {
            break;
        } else if (retries == 2) {
            return MAX17048_RESET_ERROR;
        }
    }

    // Disable sleep
    uint8_t buf[] = {0x00};
    int status = i2c_read_from_register(i2c, MAX17048_I2C_ADDR, REG_MODE, buf, 1);
    if (status != 0) {
        return MAX17048_READ_ERROR;
    }
    // Write a 0 to the 5th bit (EnSleep)
    buf[0] &= ~(1 << 5);
    status = i2c_write_to_register(i2c, MAX17048_I2C_ADDR, REG_MODE, buf, 2);
    if (status != 0) { return MAX17048_WRITE_ERROR; }

    // Set sleep to 0 - reuse buf
    uint8_t buf2[] = {0, 0};
    status = i2c_read_from_register(i2c, MAX17048_I2C_ADDR, REG_CONFIG, buf, 2);
    if (status != 0) { return MAX17048_READ_ERROR; }
    // Write a 0 to the 7th bit (Sleep)
    buf[1] &= ~(1 << 7);
    status = i2c_write_to_register(i2c, MAX17048_I2C_ADDR, REG_MODE, buf, 2);
    if (status != 0) { return MAX17048_WRITE_ERROR; }

    return 0;
}

/*
* Internal function
* Check if device is ready to communicate - read version
* According to adafruit lib - Chip ID = 0xFF and Version = 0xFFFF if no battery is attached
*/
int max17048IsDeviceReady(i2c_inst_t *i2c) {
    uint8_t buf[2];

    int status = i2c_read_from_register(i2c, MAX17048_I2C_ADDR, REG_VERSION, buf, 2);
    if (status != 0) {
        return MAX17048_READ_ERROR;
    }

    uint16_t version = buf[0] | buf[1] << 8;
    logln_error("Version: %d, if: %d", version, version & 0xFF0);
    if (version & 0xFF0 == 0x0010) {
        logln_error("Version right: %d", version);
        return 0;
    } else {
        logln_error("Version wrong: %d", version);
        return MAX17048_DEVICE_NOT_READY_ERROR;
    }
}

int max17048CellVoltage(i2c_inst_t *i2c, float *voltage_out) {
    /*int status = max17048IsDeviceReady(i2c);
    if (status != 0)
        return status;*/
    
    uint8_t buf[2];

    int status = i2c_read_from_register(i2c, MAX17048_I2C_ADDR, REG_VCELL, buf, 2);
    if (status != 0) {
        return MAX17048_VALUE_READ_ERROR;
    }
    *voltage_out = ((buf[0] | buf[1] << 8) * 78.125) / 1000000;

    return 0;
}

// For use in a packet - less room - conversion done on the ground
// Returns 2 uint8_ts
int max17048CellVoltageRaw(i2c_inst_t *i2c, uint8_t *voltage_out) {
    int status = max17048IsDeviceReady(i2c);
    if (status != 0)
        return status;

    status = i2c_read_from_register(i2c, MAX17048_I2C_ADDR, REG_VCELL, voltage_out, 2);
    if (status != 0) {
        return MAX17048_VALUE_READ_ERROR;
    }

    return 0;
}

int max17048CellPercentage(i2c_inst_t *i2c, float *percentage_out) {
    /*int status = max17048IsDeviceReady(i2c);
    if (status != 0)
        return status;*/

    uint8_t buf[2];

    int status = i2c_read_from_register(i2c, MAX17048_I2C_ADDR, REG_SOC, buf, 2);
    if (status != 0) {
        return MAX17048_VALUE_READ_ERROR;
    }

    *percentage_out = (buf[0] | buf[1] << 8) / 256.0;

    return 0;
}

int max17048CellPercentageRaw(i2c_inst_t *i2c, uint8_t *percentage_out) {
    int status = max17048IsDeviceReady(i2c);
    if (status != 0)
        return status;

    uint8_t buf[2];

    status = i2c_read_from_register(i2c, MAX17048_I2C_ADDR, REG_SOC, percentage_out, 2);
    if (status != 0) {
        return MAX17048_VALUE_READ_ERROR;
    }


    return 0;
}
