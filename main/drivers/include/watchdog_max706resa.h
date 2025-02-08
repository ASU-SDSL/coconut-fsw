#pragma once

#define WDI_PIN 21 // from FCR schematic

// Init gpio and set high
void initialize_watchdog();

// Set WDI pin high or low - state = 0 for low, 1 for high
void set_watchdog_gpio(int state);
