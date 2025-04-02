#include "hardware/gpio.h"
#include "steve.h"

#define POWER_PIN 20

void buzzer_init() {
  gpio_set_dir(POWER_PIN, GPIO_OUT);
}

void buzzer_beep_job() {
  schedule_delayed_job_ms("Buzzer beep on", &beep_on, 10);
  schedule_delayed_job_ms("Buzzer beep off", &beep_on, 10);
}

static void beep_on() {
  gpio_put(POWER_PIN, 1);
} 

static void beep_off() {
  gpio_put(POWER_PIN, 0);
}
