#include "FreeRTOS.h"

#include "hardware/gpio.h"
#include "steve.h"
#include "log.h"

#define POWER_PIN 4

void buzzer_beep_job() {
  logln_info("Beep!"); 
  gpio_init(POWER_PIN); 
  gpio_set_dir(POWER_PIN, GPIO_OUT);

  gpio_put(POWER_PIN, 1);
  vTaskDelay(pdMS_TO_TICKS(1000));
  gpio_put(POWER_PIN, 0);
} 
