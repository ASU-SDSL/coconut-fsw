#include <stdint.h>
#include <stdio.h>

#include "FreeRTOS.h"
#include "queue.h"
#include "task.h"

#include "state.h"
#include "telemetry.h"
#include "ina219.h"
#include "mag_lis3mdltr.h"
#include "rtc_ds3231.h"
#include "timing.h"
#include "vega_ant.h"
#include "heartbeat_job.h"
#include "radio.h"
#include "max17048.h"
#include "filesystem.h"

// Size (bytes) that a heartbeat telemetry file can be until a new one should be used
// If writing a new telemetry entry causes the file to go over this amount, a new file is made
#define MAX_HB_TLM_FILE_SIZE 200

/* 
*  Keep a /tlm directory where:
*  - /tlm/file_counter.bin file contains the index of the next file to be created - 2 bytes
*  - /tlm/0.bin, /tlm/1.bin, ... /tlm/65535 files contain the telemetry data, most recent of which is the number.bin contained in the file_counter.txt file
*/
void log_heartbeat_tlm(heartbeat_telemetry_t payload) {

    // See if tlm directory exists
    if (!dir_exists("/tlm")) {
        make_dir("/tlm");
    }
    // See if f_count.bin exists, if not create it and start the index at 0
    if (!file_exists("/tlm/f_count.bin")) {
        // If the directory exists but the file counter file doesn't, create it and write a 0 to it
        //touch("/tlm/f_count.bin");

        char data[] = {0, 0};
        write_file("/tlm/f_count.bin", data, 2, false); // create and write 0 to the file_counter.txt file

        logln_info("TLM dir:");
        list_dir("/tlm");
    }

    // Read the file counter
    char file_counter_buf[2];
    size_t bytes_read = read_file("/tlm/f_count.bin", file_counter_buf, sizeof(file_counter_buf));
    if (bytes_read != 2) {
        logln_error("Error reading f_count.bin");
        return;
    }

    uint16_t file_count = (file_counter_buf[0] << 8) | file_counter_buf[1];
    logln_info("Heartbeat tlm file count: %d", file_count);
    
    // Write to this indexed file
    char filename[20];
    snprintf(filename, sizeof(filename), "%d.bin", file_count);
    logln_info("Writing to file: %s", filename);
    write_file(filename, (char*)&payload, sizeof(payload), true); // append - this will eaither make the file if it does not exist or append to it if it does

    // Check file size to see if it is time to make a new file. If writing another hb to the file makes it go over the limit, 
    FILINFO file_info;
    stat(filename, &file_info);
    if (file_info.fsize + sizeof(heartbeat_telemetry_t) > MAX_HB_TLM_FILE_SIZE) {
        // Increase f_count.bin by 1 for next time
        file_count += 1;
        char new_file_counter_buf[2] = {(file_count >> 8) & 0xFF, file_count & 0xFF};
        write_file("/tlm/f_count.bin", new_file_counter_buf, 2, false); // overwrite the file counter with the new value
        logln_info("New tlm file should be created");
    }

    logln_info("\nTLM DIR:");
    list_dir("/tlm");
    
}

void heartbeat_telemetry_job(void* unused) {
    // Create heartbeat struct
    heartbeat_telemetry_t payload;

    logln_info("%s", get_current_task_name());

    // State data
    payload.state = (uint8_t)g_payload_state;
    payload.uptime = (uint32_t)get_uptime();

    // i2c instance
    i2c_inst_t *i2c = i2c1;

    // MAX17048 data
    float max17048Voltage;
    int status = max17048CellVoltage(i2c, &max17048Voltage);
    if (status != 0) {
        logln_error("Error reading MAX17048 voltage: %d", status);
        max17048Voltage = -1;
    }

    float max17048Percentage;
    status = max17048CellPercentage(i2c, &max17048Percentage);
    if (status != 0) {
        logln_error("Error reading MAX17048 percentage: %d", status);
        max17048Percentage = -1;
    }

    payload.max17048Voltage = max17048Voltage;
    payload.max17048Percentage = max17048Percentage;

    // For testing if needed
    //logln_info("MAX17048 Voltage: %f, Percentage: %f\n", max17048Voltage, max17048Percentage);

    // timestamp
    uint8_t rtcbuf;
    if(!rtc_get_hour(i2c, &rtcbuf)) payload.hour = rtcbuf; 
    else payload.hour = UINT8_MAX;
    if(!rtc_get_minute(i2c, &rtcbuf)) payload.minute = rtcbuf; 
    else payload.minute = UINT8_MAX; 
    if(!rtc_get_second(i2c, &rtcbuf)) payload.second = rtcbuf;
    else payload.second = UINT8_MAX;
    if(!rtc_get_month(i2c, &rtcbuf)) payload.month = rtcbuf;
    else payload.month = UINT8_MAX;
    if(!rtc_get_date(i2c, &rtcbuf)) payload.date = rtcbuf;
    else payload.date = UINT8_MAX;
    if(!rtc_get_year(i2c, &rtcbuf)) payload.year = rtcbuf;
    else payload.year = UINT8_MAX;
    float rtcTemp = 5500;
    if(!rtc_get_temp(i2c, &rtcTemp)) payload.rtcTemp = rtcTemp;

    // ina0 data
    uint16_t ina0buf;
    if(!getVShunt_raw(i2c, INA0_ADDR, &ina0buf)) payload.ina0_shunt = ina0buf;
    else payload.ina0_shunt = UINT16_MAX;
    if(!getVBus_raw(i2c, INA0_ADDR, &ina0buf)) payload.ina0_vbus = ina0buf;
    else payload.ina0_vbus = UINT16_MAX; 
    if(!getCurrent_raw(i2c, INA0_ADDR, &ina0buf)) payload.ina0_current = ina0buf; 
    else payload.ina0_current = UINT16_MAX; 
    if(!getPower_raw(i2c, INA0_ADDR, &ina0buf)) payload.ina0_power = ina0buf;
    else payload.ina0_power = UINT16_MAX; 

    /*logln_info("INA0 shunt: %ld", payload.ina0_shunt);
    logln_info("INA0 vbus: %ld", payload.ina0_vbus);
    logln_info("INA0 current: %ld", payload.ina0_current);
    logln_info("INA0 power: %ld", payload.ina0_power);*/

    // ina1 data
    uint16_t ina1buf;
    if(!getVShunt_raw(i2c, INA2_ADDR, &ina1buf)) payload.ina1_shunt = ina1buf;
    else payload.ina1_shunt = UINT16_MAX;
    if(!getVBus_raw(i2c, INA2_ADDR, &ina1buf)) payload.ina1_vbus = ina1buf;
    else payload.ina1_vbus = UINT16_MAX; 
    if(!getCurrent_raw(i2c, INA2_ADDR, &ina1buf)) payload.ina1_current = ina1buf; 
    else payload.ina1_current = UINT16_MAX; 
    if(!getPower_raw(i2c, INA2_ADDR, &ina1buf)) payload.ina1_power = ina1buf;
    else payload.ina1_power = UINT16_MAX; 

    // INA2 data
    uint16_t ina2buf;
    if(!getVShunt_raw(i2c, INA3_ADDR, &ina2buf)) payload.ina2_shunt = ina2buf;
    else payload.ina2_shunt = UINT16_MAX;
    if(!getVBus_raw(i2c, INA3_ADDR, &ina2buf)) payload.ina2_vbus = ina2buf;
    else payload.ina2_vbus = UINT16_MAX; 
    if(!getCurrent_raw(i2c, INA3_ADDR, &ina2buf)) payload.ina2_current = ina2buf; 
    else payload.ina2_current = UINT16_MAX; 
    if(!getPower_raw(i2c, INA3_ADDR, &ina2buf)) payload.ina2_power = ina2buf;
    else payload.ina2_power = UINT16_MAX; 

    // INA3 data
    uint16_t ina3buf;
    if(!getVShunt_raw(i2c, INA4_ADDR, &ina3buf)) payload.ina3_shunt = ina3buf;
    else payload.ina3_shunt = UINT16_MAX;
    if(!getVBus_raw(i2c, INA4_ADDR, &ina3buf)) payload.ina3_vbus = ina3buf;
    else payload.ina3_vbus = UINT16_MAX; 
    if(!getCurrent_raw(i2c, INA4_ADDR, &ina3buf)) payload.ina3_current = ina3buf; 
    else payload.ina3_current = UINT16_MAX; 
    if(!getPower_raw(i2c, INA4_ADDR, &ina3buf)) payload.ina3_power = ina3buf;
    else payload.ina3_power = UINT16_MAX; 

    // ina4 data
    uint16_t ina4buf;
    if(!getVShunt_raw(i2c, INA5_ADDR, &ina4buf)) payload.ina4_shunt = ina4buf;
    else payload.ina4_shunt = UINT16_MAX;
    if(!getVBus_raw(i2c, INA5_ADDR, &ina4buf)) payload.ina4_vbus = ina4buf;
    else payload.ina4_vbus = UINT16_MAX; 
    if(!getCurrent_raw(i2c, INA5_ADDR, &ina4buf)) payload.ina4_current = ina4buf; 
    else payload.ina4_current = UINT16_MAX; 
    if(!getPower_raw(i2c, INA5_ADDR, &ina4buf)) payload.ina4_power = ina4buf;
    else payload.ina4_power = UINT16_MAX;

    // ina5 data
    uint16_t ina5buf;
    if(!getVShunt_raw(i2c, INA5_ADDR, &ina5buf)) payload.ina5_shunt = ina5buf;
    else payload.ina5_shunt = UINT16_MAX;
    if(!getVBus_raw(i2c, INA5_ADDR, &ina5buf)) payload.ina5_vbus = ina5buf;
    else payload.ina5_vbus = UINT16_MAX; 
    if(!getCurrent_raw(i2c, INA5_ADDR, &ina5buf)) payload.ina5_current = ina5buf; 
    else payload.ina5_current = UINT16_MAX; 
    if(!getPower_raw(i2c, INA5_ADDR, &ina5buf)) payload.ina5_power = ina5buf;
    else payload.ina5_power = UINT16_MAX;

    // Mag data
    int16_t magbuf; 
    if(!mag_get_x(i2c, &magbuf)) payload.mag_x = magbuf;
    else payload.mag_x = INT16_MAX; 
    if(!mag_get_y(i2c, &magbuf)) payload.mag_y = magbuf;
    else payload.mag_y = INT16_MAX; 
    if(!mag_get_z(i2c, &magbuf)) payload.mag_z = magbuf; 
    else payload.mag_z = INT16_MAX; 
    if(!mag_get_temp(i2c, &magbuf)) payload.mag_temp = magbuf; 
    else payload.mag_temp = INT16_MAX; 

    uint8_t vega_ant_buf;
    if(!vega_ant_status(i2c, &vega_ant_buf)) payload.vega_ant_status = vega_ant_buf;
    else payload.vega_ant_status = UINT8_MAX;

    // radio status reporting
    payload.which_radio = radio_which(); 
    payload.rfm_state = radio_get_RFM_state(); 
    payload.sx_state = radio_get_SX_state(); 
    
    // Send it
    logln_info("Telem size: %d", sizeof(payload));
    send_telemetry(HEARTBEAT, (char*)&payload, sizeof(payload));
    log_heartbeat_tlm(payload);

    iteration_counter += 1;
    logln_info("Heartbeat %ld - uptime: %d", iteration_counter, (uint32_t)get_uptime());
}
