#pragma once

#include <stdint.h>

#include "rtc_ds3231.h"
#include <FreeRTOS.h>

void set_rtc_job(void* args); 