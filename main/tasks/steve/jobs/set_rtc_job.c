#include "set_rtc_job.h"
#include "log.h"

void set_rtc_job(void* args){
    i2c_inst_t *i2c = i2c1; 
    uint8_t* time = (uint8_t*)args; 

    // logln_info("%d %d %d %d %d %d", time[0], time[1], time[2], time[3], time[4], time[5]); 
    uint8_t res = rtc_set_time(i2c, time[0], time[1], time[2], time[3], time[4], time[5]);
    // uint8_t res = rtc_set_time(i2c, 1, 1, 1, 1, 1, 1);

    vPortFree(args); 
    // logln_info("RTC Set with code: %d", res); 
}