#pragma once


// FreeRTOS
#include <FreeRTOS.h>
#include <task.h>
#include <queue.h>
// C
#include <stdbool.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <time.h>
#include "pico/stdlib.h"

// Project headers
#include "gse.h"


#ifndef SIMULATOR
#include "radio.h"
#endif

#include "command.h"
#include "telemetry.h"
#include "log.h"
#include "steve.h"
#include "filesystem.h"

// Function defs
int main();

