#pragma once

#define HEARTBEAT_JOB_NAME "heartbeat_telemetry"
#define HEARTBEAT_TELEMETRY_DEFAULT_INTERVAL 5

uint32_t iteration_counter;

/**
 * @brief Collects sensor information and transmits it as a heartbeat packet 
 * 
 * @param unused 
 */
void heartbeat_telemetry_job(void* unused);
