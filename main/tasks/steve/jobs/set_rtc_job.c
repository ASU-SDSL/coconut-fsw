#include "set_rtc_job.h"


void set_rtc_job(void* args){
    i2c_inst_t *i2c = i2c1; 
    uint8_t* time = (uint8_t*)args; 

    rtc_set_time(i2c, time[0], time[1], time[2], time[3], time[4], time[5]);

    vPortFree(args); 
}