#include "heartbeat_job.h"


void heartbeat_telemetry_job(void* unused) {
    // Create heartbeat struct
    heartbeat_telemetry_t payload;

    // State data
    payload.state = g_payload_state;
    payload.uptime = get_uptime();

    // i2c instance
    i2c_inst_t *i2c = i2c0;

    // timestamp
    uint8_t rtcbuf; 
    if(!rtc_get_hour(i2c, &rtcbuf)) payload.hour = rtcbuf; 
    else payload.hour = 0;
    if(!rtc_get_minute(i2c, &rtcbuf)) payload.minute = rtcbuf; 
    else payload.minute = 0; 
    if(!rtc_get_second(i2c, &rtcbuf)) payload.second = rtcbuf;
    else payload.second = 0;
    if(!rtc_get_month(i2c, &rtcbuf)) payload.month = rtcbuf;
    else payload.month = 0;
    if(!rtc_get_date(i2c, &rtcbuf)) payload.date = rtcbuf;
    else payload.date = 0;
    if(!rtc_get_year(i2c, &rtcbuf)) payload.year = rtcbuf;
    else payload.year = 0;
    float rtcTemp; 
    if(!rtc_get_temp(i2c, &rtcTemp)) payload.rtcTemp = rtcTemp;
    else payload.rtcTemp = rtcTemp; 

    // EPS data
    uint16_t epsbuf;
    if(!getVShunt_raw(i2c, &epsbuf)) payload.eps_shunt = epsbuf;
    else payload.eps_shunt = 0;
    if(!getVBus_raw(i2c, &epsbuf)) payload.eps_vbus = epsbuf;
    else payload.eps_vbus = 0; 
    if(!getCurrent_raw(i2c, &epsbuf)) payload.eps_current = epsbuf; 
    else payload.eps_current = 0; 
    if(!getPower_raw(i2c, &epsbuf)) payload.eps_power = epsbuf;
    else payload.eps_power = 0; 

    // Mag data
    int16_t magbuf; 
    if(!mag_get_x(i2c, magbuf)) payload.mag_x = magbuf;
    else payload.mag_x = 0; 
    if(!mag_get_y(i2c, magbuf)) payload.mag_y = magbuf;
    else payload.mag_y = 0; 
    if(!mag_get_z(i2c, magbuf)) payload.mag_z = magbuf; 
    else payload.mag_z = 0; 
    if(!mag_get_temp(i2c, magbuf)) payload.mag_temp = magbuf; 
    else payload.mag_temp = 0; 

    // Send it
    send_telemetry(HEARTBEAT, (char*)&payload, sizeof(payload));

    // Logging
    iteration_counter += 1;
    logln_info("Heartbeat %ld\n", iteration_counter);
}