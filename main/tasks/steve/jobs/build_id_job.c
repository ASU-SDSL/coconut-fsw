#include "build_id_job.h"
#include "log.h"

/*

 should print date and time once at the beginning of run
*/

void build_id_job(void *unused) {
    logln_info("Firmware build: %s  %s", __DATE__, __TIME__);
}

