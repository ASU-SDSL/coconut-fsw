#pragma once

#include <FreeRTOS.h>
#include "queue.h"

#include "filesystem.h"
#include "radio.h"
#include "log.h"

#define SDJ_CHUNK_SIZE 128 

typedef struct {
  char *file_name
} simple_downlink_args_t; 

void simple_downlink_job(void * args);