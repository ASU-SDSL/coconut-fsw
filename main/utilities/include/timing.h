#pragma once

#include "FreeRTOS.h"
#include "semphr.h"
#include "pico/types.h"
#include "pico/time.h"

// Constants
#define MS_IN_SEC 1000
#define SECS_IN_MIN 60

// Function defs
TickType_t ms_to_ticks(unsigned long ms);
unsigned long ticks_to_ms(TickType_t ms);
unsigned long secs_to_ms(unsigned long secs);
unsigned long ms_to_secs(unsigned long ms);
unsigned long mins_to_secs(unsigned long mins);
unsigned long secs_to_mins(unsigned long mins);
TickType_t get_uptime();

// persistent timing 
extern SemaphoreHandle_t epoch_time_mutex;
extern uint64_t epoch_time; // second resolution 

uint64_t get_epoch_time(); 
void update_epoch_time(uint8_t year, uint8_t month, uint8_t date, uint8_t hour, uint8_t minute, uint8_t second); 

uint64_t time_since_ms(uint64_t past_time); 

uint64_t timing_now(); 