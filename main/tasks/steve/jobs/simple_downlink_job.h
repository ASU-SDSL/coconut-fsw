#pragma once

#include <FreeRTOS.h>
#include "queue.h"

#include "filesystem.h"
#include "radio.h"
#include "log.h"

#define SDJ_CHUNK_SIZE 128 

void simple_downlink_job(void * args);