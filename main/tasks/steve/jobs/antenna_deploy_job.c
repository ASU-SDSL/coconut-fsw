#include "vega_ant.h"
#include "log.h"

void deploy_antenna_job() {
    logln_info("Deploying antenna...");

    // Antenna is on bus 1
    i2c_inst_t *i2c = i2c1;
    int status = vega_ant_deploy(i2c);

    if (status != 0) {
        logln_error("ERROR in the antenna deployment with code '%d'", status);
    }

    logln_info("Deploy antenna task complete");
}
