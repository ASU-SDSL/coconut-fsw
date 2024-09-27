#pragma once

#include <stdbool.h>
#include <stdlib.h>
//#include <stdio.h>
#include <string.h>
#include <time.h>
#include "pico/stdlib.h"

#include <FreeRTOS.h>
#include <task.h>
#include <queue.h>


#include "gse.h"
#include "command.h"
#include "telemetry.h"
#include "log.h"
#include "steve.h"
#include "filesystem.h"

#ifndef SIMULATOR
#include "radio.h"
#endif

// Function defs
int main();

