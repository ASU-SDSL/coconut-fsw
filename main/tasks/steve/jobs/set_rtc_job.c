#include "set_rtc_job.h"
#include "log.h"
#include "watchdog.h"
#include "radio.h"
#include "timing.h"
#include "filesystem.h"
#include "rtc_ds3231.h"
#include <string.h>

void set_rtc_job(void* args){
    i2c_inst_t *i2c = i2c1; 
    uint8_t* time = (uint8_t*)args; 

    // logln_info("%d %d %d %d %d %d", time[0], time[1], time[2], time[3], time[4], time[5]); 
    uint8_t res = rtc_set_time(i2c, time[0], time[1], time[2], time[3], time[4], time[5]);
    // uint8_t res = rtc_set_time(i2c, 1, 1, 1, 1, 1, 1);

    // update epoch time
    if(res == 0){
        update_epoch_time(time[0], time[1], time[2], time[3], time[4], time[5]);

        // redo some of radio_flag_valid_packet as this commend will mess up the real time clock 
        uint64_t new_time = timing_now();
        set_radio_last_received_time(new_time);  

        logln_info("Updating last received time to %llu", new_time);
        char buffer[sizeof(uint64_t)]; 
        memcpy(buffer, &new_time, sizeof(uint64_t)); 
        write_file(RADIO_STATE_FILE_NAME, buffer, sizeof(uint64_t), false); 
    }

    vPortFree(args); 
    // logln_info("RTC Set with code: %d", res); 
}