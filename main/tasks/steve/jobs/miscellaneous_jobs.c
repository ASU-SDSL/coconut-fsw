#include "hardware/gpio.h"
#include "steve.h"

#define POWER_PIN 4

static void beep() {
  gpio_put(POWER_PIN, 1);
  vTaskDelay(100);
  gpio_put(POWER_PIN, 0);
} 

void buzzer_init() {
  gpio_set_dir(POWER_PIN, GPIO_OUT);
}

void buzzer_beep_job() {
  schedule_delayed_job_ms("Buzzer beep", &beep_on, 10);
}
