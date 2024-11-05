#include "heartbeat_job.h"


void heartbeat_telemetry_job(void* unused) {
    // Create heartbeat struct
    heartbeat_telemetry_t payload;

    // State data
    payload.state = (uint8_t)g_payload_state;
    payload.uptime = (uint32_t)get_uptime();

    logln_info("Uptime hex from struct: %#010x\n", payload.uptime);
    logln_info("Uptime hex from func: %#050x\n", (uint32_t)get_uptime());

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
    else payload.rtcTemp = rtcTemp; 

    // EPS data
    uint16_t epsbuf;
    if(!getVShunt_raw(i2c, &epsbuf)) payload.eps_shunt = epsbuf;
    else payload.eps_shunt = UINT16_MAX;
    if(!getVBus_raw(i2c, &epsbuf)) payload.eps_vbus = epsbuf;
    else payload.eps_vbus = UINT16_MAX; 
    if(!getCurrent_raw(i2c, &epsbuf)) payload.eps_current = epsbuf; 
    else payload.eps_current = UINT16_MAX; 
    if(!getPower_raw(i2c, &epsbuf)) payload.eps_power = epsbuf;
    else payload.eps_power = UINT16_MAX; 

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

    uint8_t* ptr = (uint8_t*) &payload;
    for(int i = 0; i < sizeof(payload); i++) {
        log_gen("%02x", ptr[i]);
    }
    log_gen("\n");

    // Logging
    iteration_counter += 1;
    logln_info("Heartbeat %ld - uptime: %d", iteration_counter, (uint32_t)get_uptime());
}