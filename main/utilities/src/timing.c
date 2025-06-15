#include "FreeRTOS.h"
#include "task.h"

#include "timing.h"

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

uint32_t time_between(uint32_t before, uint32_t after) {
    if(after > before) return after - before;
    else return UINT32_MAX - before + after; // catch rollover 
}