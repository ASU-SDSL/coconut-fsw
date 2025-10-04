#include "vega_ant.h"
#include "log.h"
#include "steve.h"

void deploy_antenna_job() {
    logln_info("Deploying antenna...");

    // Antenna is on bus 1
    i2c_inst_t *i2c = i2c1;
    int status = vega_ant_deploy(i2c);

    if (status != 0) {
        logln_error("ERROR in the antenna deployment with code '%d'", status);
    }

    uint8_t antenna_switch_status = 0; 
    status = vega_ant_status(i2c, &antenna_switch_status); 
    if (status != 0 || antenna_switch_status != 0) {
        logln_error("Antenna switch status %d, retrying in 10 min");
        schedule_delayed_job_mins("DEPLOY_ANTENNA", &deploy_antenna_job, 10);
    } else {
        logln_info("Deploy antenna task complete");
    }
}
