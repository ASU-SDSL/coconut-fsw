#ifndef RADIO_FILE_DEFINED
#define RADIO_FILE_DEFINED
#include <log.h>
#include "FreeRTOS.h"


void radio_task(void* unused_arg);
void transmit_task(void* unused_arg);


#endif /* !RADIO_FILE_DEFINED */