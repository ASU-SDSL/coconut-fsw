#include "FreeRTOS.h"
#include "task.h"

#include "timing.h"
#include "log.h"

#include <time.h>

static bool epoch_time_updated = false; 

TickType_t ms_to_ticks(unsigned long ms) {
    return pdMS_TO_TICKS(ms);
}

unsigned long ticks_to_ms(TickType_t ticks) {
    return pdTICKS_TO_MS(ticks);
}

unsigned long secs_to_ms(unsigned long secs) {
    return secs * MS_IN_SEC;
}

unsigned long ms_to_secs(unsigned long ms) {
    return ms / MS_IN_SEC;
}

unsigned long secs_to_mins(unsigned long secs) {
    return secs / SECS_IN_MIN;
}

unsigned long mins_to_secs(unsigned long mins) {
    return mins * SECS_IN_MIN;
}

TickType_t get_uptime() {
    // TODO: Maybe get an RTC instead of using CPU ticks
    return xTaskGetTickCount();
}

uint32_t time_between(uint32_t after, uint32_t before) {
    if(after > before) return after - before;
    else return (UINT32_MAX - before) + after; // catch rollover 
}

// persistent timing 
uint64_t epoch_time = 0; 
SemaphoreHandle_t epoch_time_mutex; 

uint64_t get_epoch_time(){
    uint64_t value; 
    
    // check for null to just go for it and hope 
    if(epoch_time_mutex == NULL || xSemaphoreTake(epoch_time_mutex, portMAX_DELAY) == pdTRUE){
        value = epoch_time; 
        xSemaphoreGive(epoch_time_mutex); 
    } else {
        return 0; 
    }

    return value; 
}

void update_epoch_time(uint8_t year, uint8_t month, uint8_t date, uint8_t hour, uint8_t minute, uint8_t second){
    epoch_time_updated = true;

    struct tm temp = {
        .tm_year = year + 100,  
        .tm_mon = month, 
        .tm_mday = date, 
        .tm_hour = hour, 
        .tm_min = minute, 
        .tm_sec = second
    }; 


    time_t temp_time = mktime(&temp); 

    if(xSemaphoreTake(epoch_time_mutex, portMAX_DELAY) == pdTRUE){
        epoch_time = ((uint64_t)(temp_time)) * 1000; // epoch time should be in seconds 
        xSemaphoreGive(epoch_time_mutex);
    } else {
        logln_error("Epoch time update failed");
    }
}

// timing with back up of since boot
// using the size of the time_t to determine if the readings are unix epoch or since boot 
uint64_t time_since_ms(uint64_t past_time){
  // less than 5 years worth of milliseconds -> past was a since boot 
  if(past_time < (157784760000L)){
    return (uint64_t) time_between(to_ms_since_boot(get_absolute_time()), past_time); 
  }
  // otherwise past was an epoch time 
  else {
    return get_epoch_time() - past_time; 
  }
}

uint64_t timing_now(){
    if(epoch_time_updated) { 
        return get_epoch_time(); 
    } else {
        return (uint64_t) to_ms_since_boot(get_absolute_time()); 
    }
}