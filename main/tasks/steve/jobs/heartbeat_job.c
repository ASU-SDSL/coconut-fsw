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
#include "hb_tlm_log.h"
#include "ds18b20.h"
#include "command.h"
#include "filesystem.h"

static uint32_t bootcount;
static bool bootcountset;

void heartbeat_telemetry_job(void* unused) {
    // Create heartbeat struct
    heartbeat_telemetry_t payload;
    
    if(!bootcountset){
        //check if bootcount file exist
        if (file_exists("boot.bin")){
                if(read_file("boot.bin", (char*)&bootcount, sizeof(bootcount)) > 0){
                    bootcount++;
                    //write to file
                    write_file("boot.bin", (char*)&bootcount, sizeof(bootcount), false);
                    bootcountset = true;
                }
                else{
                    //if bootcount file exists but read failed, set bootcount to 0
                    delete_file("boot.bin");
                    logln_error("Failed to read bootcount file, setting bootcount to 0");
                    bootcount = 0;
                    write_file("boot.bin", (char*)&bootcount, sizeof(bootcount), false);
                    bootcountset = true;
                }
        }
        else{
            //set bootcount to 0 if no file exist 
            bootcount = 0;
            //write to file
            write_file("boot.bin", (char*)&bootcount, sizeof(bootcount), false);
            bootcountset = true;
        }
    }
        
    // logln_info("%s", get_current_task_name());

    //Incorporate callsign, using Tyler's for now.
    payload.callsign[0] = 'K';
    payload.callsign[1] = 'K';
    payload.callsign[2] = '7';
    payload.callsign[3] = 'L';
    payload.callsign[4] = 'T';
    payload.callsign[5] = 'W';
    payload.callsign[6] = '\0';
    payload.callsign[7] = '\0';



    // State data
    payload.state = (uint8_t)g_payload_state;
    payload.uptime = (uint32_t)get_uptime();

    // i2c instance
    i2c_inst_t *i2c = i2c1;

    // start pio conversion 
    uint8_t ds18b20_conversion_res = ds18b20_start_conversion(); 

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

    // use timestamp to update absolute time (check for failure first)
    if(payload.year != UINT8_MAX && payload.month != UINT8_MAX && 
        payload.date != UINT8_MAX && payload.hour != UINT8_MAX && 
        payload.minute != UINT8_MAX && payload.second != UINT8_MAX) {
        update_epoch_time(payload.year, payload.month, payload.date, payload.hour, payload.minute, payload.second); 
    }

    // ina0 data
    config(i2c, INA0_ADDR); 
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
    config(i2c, INA1_ADDR); 
    uint16_t ina1buf;
    if(!getVShunt_raw(i2c, INA1_ADDR, &ina1buf)) payload.ina1_shunt = ina1buf;
    else payload.ina1_shunt = UINT16_MAX;
    if(!getVBus_raw(i2c, INA1_ADDR, &ina1buf)) payload.ina1_vbus = ina1buf;
    else payload.ina1_vbus = UINT16_MAX; 
    if(!getCurrent_raw(i2c, INA1_ADDR, &ina1buf)) payload.ina1_current = ina1buf; 
    else payload.ina1_current = UINT16_MAX; 
    if(!getPower_raw(i2c, INA1_ADDR, &ina1buf)) payload.ina1_power = ina1buf;
    else payload.ina1_power = UINT16_MAX; 

    // INA2 data
    config(i2c, INA2_ADDR); 
    uint16_t ina2buf;
    if(!getVShunt_raw(i2c, INA2_ADDR, &ina2buf)) payload.ina2_shunt = ina2buf;
    else payload.ina2_shunt = UINT16_MAX;
    if(!getVBus_raw(i2c, INA2_ADDR, &ina2buf)) payload.ina2_vbus = ina2buf;
    else payload.ina2_vbus = UINT16_MAX; 
    if(!getCurrent_raw(i2c, INA2_ADDR, &ina2buf)) payload.ina2_current = ina2buf; 
    else payload.ina2_current = UINT16_MAX; 
    if(!getPower_raw(i2c, INA2_ADDR, &ina2buf)) payload.ina2_power = ina2buf;
    else payload.ina2_power = UINT16_MAX; 

    // INA3 data
    config(i2c, INA3_ADDR);
    uint16_t ina3buf;
    if(!getVShunt_raw(i2c, INA3_ADDR, &ina3buf)) payload.ina3_shunt = ina3buf;
    else payload.ina3_shunt = UINT16_MAX;
    if(!getVBus_raw(i2c, INA3_ADDR, &ina3buf)) payload.ina3_vbus = ina3buf;
    else payload.ina3_vbus = UINT16_MAX; 
    if(!getCurrent_raw(i2c, INA3_ADDR, &ina3buf)) payload.ina3_current = ina3buf; 
    else payload.ina3_current = UINT16_MAX; 
    if(!getPower_raw(i2c, INA3_ADDR, &ina3buf)) payload.ina3_power = ina3buf;
    else payload.ina3_power = UINT16_MAX; 

    // ina4 data
    config(i2c, INA4_ADDR); 
    uint16_t ina4buf;
    if(!getVShunt_raw(i2c, INA4_ADDR, &ina4buf)) payload.ina4_shunt = ina4buf;
    else payload.ina4_shunt = UINT16_MAX;
    if(!getVBus_raw(i2c, INA4_ADDR, &ina4buf)) payload.ina4_vbus = ina4buf;
    else payload.ina4_vbus = UINT16_MAX; 
    if(!getCurrent_raw(i2c, INA4_ADDR, &ina4buf)) payload.ina4_current = ina4buf; 
    else payload.ina4_current = UINT16_MAX; 
    if(!getPower_raw(i2c, INA4_ADDR, &ina4buf)) payload.ina4_power = ina4buf;
    else payload.ina4_power = UINT16_MAX;

    // ina5 data
    config(i2c, INA5_ADDR); 
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

    if(ds18b20_conversion_res == 0){
        payload.temp_eps = ds18b20_read_temp(DS18B_ROMCODE_EPS); 
        payload.temp_Xm = ds18b20_read_temp(DS18B_ROMCODE_XM); 
        payload.temp_Xp = ds18b20_read_temp(DS18B_ROMCODE_XP);
        payload.temp_Ym = ds18b20_read_temp(DS18B_ROMCODE_YM);
        payload.temp_Yp = ds18b20_read_temp(DS18B_ROMCODE_YP);
        payload.temp_Zm = ds18b20_read_temp(DS18B_ROMCODE_ZM);
        payload.temp_Zp = ds18b20_read_temp(DS18B_ROMCODE_ZP); 
    } else {
        payload.temp_eps = INT16_MAX; 
        payload.temp_Xm = INT16_MAX; 
        payload.temp_Xp = INT16_MAX; 
        payload.temp_Ym = INT16_MAX;
        payload.temp_Yp = INT16_MAX;
        payload.temp_Zm = INT16_MAX;
        payload.temp_Zp = INT16_MAX;
        logln_error("DS18B20 Temp conversion failed"); 
    }

    // radio status reporting
    payload.which_radio = radio_which(); 
    payload.rfm_state = radio_get_RFM_state(); 
    payload.sx_state = radio_get_SX_state(); 
    payload.command_count = get_command_count();
    payload.boot_count = bootcount;
    
    // Send it
    send_telemetry(HEARTBEAT_APID, (char*)&payload, sizeof(payload));
    log_heartbeat_tlm(payload);

    iteration_counter += 1;

    logln_info("Heartbeat %ld - uptime: %d", iteration_counter, (uint32_t)get_uptime());
}
