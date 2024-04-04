#include "heartbeat_job.h"

void heartbeat_telemetry_job(void* unused) {
    // Create heartbeat struct
    heartbeat_telemetry_t payload;

    // State data
    payload.state = g_payload_state;
    payload.uptime = get_uptime();

    // EPS data
    i2c_inst_t *i2c = i2c0;
    uint16_t buf;
    getVShunt_raw(i2c, &buf);
    payload.eps_shunt = buf;
    getVBus_raw(i2c, &buf);
    payload.eps_vbus = buf;
    getCurrent_raw(i2c, &buf);
    payload.eps_current = buf; 
    getPower_raw(i2c, &buf);
    payload.eps_power = buf;

    // Mag data
    payload.mag_x = get_x_output(i2c);
    payload.mag_y = get_y_output(i2c);
    payload.mag_z = get_z_output(i2c);
    payload.mag_temp = get_temp_output(i2c);

    // Send it
    send_telemetry(HEARTBEAT, (char*)&payload, sizeof(payload));

    // Logging
    iteration_counter += 1;
    logln_info("Heartbeat %ld\n", iteration_counter);
}