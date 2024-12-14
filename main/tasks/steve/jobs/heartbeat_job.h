#pragma once

#include <stdint.h>

#include "state.h"
#include "telemetry.h"
#include "eps.h"
#include "mag.h"
#include "rtc.h"
#include "vega_ant.h"
#include "radio.h"

#define HEARTBEAT_JOB_NAME "heartbeat_telemetry"
#define HEARTBEAT_TELEMETRY_DEFAULT_INTERVAL 5

uint32_t iteration_counter;

void heartbeat_telemetry_job(void* unused);
