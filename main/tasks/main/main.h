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
// Pico SDK
#include "pico/stdlib.h"
#include "pico/binary_info.h"
// Project headers
#include "gse.h"
#include "radio.h"
#include "command.h"
#include "telemetry.h"
#include "log.h"
#include "steve.h"
#include "filesystem.h"

// Function defs
int main();

