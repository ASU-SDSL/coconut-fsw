#include "heartbeat_job.h"


void heartbeat_telemetry_job(void* unused) {
    // Create heartbeat struct
    heartbeat_telemetry_t payload;
/*
    // State data
    payload.state = (uint8_t)g_payload_state;
    payload.uptime = (uint64_t)get_uptime();

    // i2c instance
    i2c_inst_t *i2c = i2c0;

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

    // INA900 data
    uint16_t ina900buf;
    if(!getVShunt_raw(i2c, INA1_ADDR, &ina900buf)) payload.ina900_shunt = ina900buf;
    else payload.ina900_shunt = UINT16_MAX;
    if(!getVBus_raw(i2c, INA1_ADDR, &ina900buf)) payload.ina900_vbus = ina900buf;
    else payload.ina900_vbus = UINT16_MAX; 
    if(!getCurrent_raw(i2c, INA1_ADDR, &ina900buf)) payload.ina900_current = ina900buf; 
    else payload.ina900_current = UINT16_MAX; 
    if(!getPower_raw(i2c, INA1_ADDR, &ina900buf)) payload.ina900_power = ina900buf;
    else payload.ina900_power = UINT16_MAX; 

    logln_info("INA1 shunt: %ld", payload.ina900_shunt);
    logln_info("INA1 vbus: %ld", payload.ina900_vbus);
    logln_info("INA1 current: %ld", payload.ina900_current);
    logln_info("INA1 power: %ld", payload.ina900_power);

    // INA1000 data
    uint16_t ina1000buf;
    if(!getVShunt_raw(i2c, INA2_ADDR, &ina1000buf)) payload.ina1000_shunt = ina1000buf;
    else payload.ina1000_shunt = UINT16_MAX;
    if(!getVBus_raw(i2c, INA2_ADDR, &ina1000buf)) payload.ina1000_vbus = ina1000buf;
    else payload.ina1000_vbus = UINT16_MAX; 
    if(!getCurrent_raw(i2c, INA2_ADDR, &ina1000buf)) payload.ina1000_current = ina1000buf; 
    else payload.ina1000_current = UINT16_MAX; 
    if(!getPower_raw(i2c, INA2_ADDR, &ina1000buf)) payload.ina1000_power = ina1000buf;
    else payload.ina1000_power = UINT16_MAX; 

    // INA1100 data
    uint16_t ina1100buf;
    if(!getVShunt_raw(i2c, INA3_ADDR, &ina1100buf)) payload.ina1100_shunt = ina1100buf;
    else payload.ina1100_shunt = UINT16_MAX;
    if(!getVBus_raw(i2c, INA3_ADDR, &ina1100buf)) payload.ina1100_vbus = ina1100buf;
    else payload.ina1100_vbus = UINT16_MAX; 
    if(!getCurrent_raw(i2c, INA3_ADDR, &ina1100buf)) payload.ina1100_current = ina1100buf; 
    else payload.ina1100_current = UINT16_MAX; 
    if(!getPower_raw(i2c, INA3_ADDR, &ina1100buf)) payload.ina1100_power = ina1100buf;
    else payload.ina1100_power = UINT16_MAX; 

    // INA1200 data
    uint16_t ina1200buf;
    if(!getVShunt_raw(i2c, INA4_ADDR, &ina1200buf)) payload.ina1200_shunt = ina1200buf;
    else payload.ina1200_shunt = UINT16_MAX;
    if(!getVBus_raw(i2c, INA4_ADDR, &ina1200buf)) payload.ina1200_vbus = ina1200buf;
    else payload.ina1200_vbus = UINT16_MAX; 
    if(!getCurrent_raw(i2c, INA4_ADDR, &ina1200buf)) payload.ina1200_current = ina1200buf; 
    else payload.ina1200_current = UINT16_MAX; 
    if(!getPower_raw(i2c, INA4_ADDR, &ina1200buf)) payload.ina1200_power = ina1200buf;
    else payload.ina1200_power = UINT16_MAX; 

    // INA1300 data
    uint16_t ina1300buf;
    if(!getVShunt_raw(i2c, INA5_ADDR, &ina1300buf)) payload.ina1300_shunt = ina1300buf;
    else payload.ina1300_shunt = UINT16_MAX;
    if(!getVBus_raw(i2c, INA5_ADDR, &ina1300buf)) payload.ina1300_vbus = ina1300buf;
    else payload.ina1300_vbus = UINT16_MAX; 
    if(!getCurrent_raw(i2c, INA5_ADDR, &ina1300buf)) payload.ina1300_current = ina1300buf; 
    else payload.ina1300_current = UINT16_MAX; 
    if(!getPower_raw(i2c, INA5_ADDR, &ina1300buf)) payload.ina1300_power = ina1300buf;
    else payload.ina1300_power = UINT16_MAX;

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

    // Send it
    send_telemetry(HEARTBEAT, (char*)&payload, sizeof(payload));

    // Logging
    iteration_counter += 1;*/
    //logln_info("Heartbeat %ld", iteration_counter);
    logln_info("TEST");
}
