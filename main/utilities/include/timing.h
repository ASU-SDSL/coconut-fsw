#pragma once

#include "FreeRTOS.h"

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