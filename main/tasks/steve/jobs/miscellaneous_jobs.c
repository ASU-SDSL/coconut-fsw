#include "FreeRTOS.h"

#include "hardware/gpio.h"
#include "pico/stdlib.h"
#include "hardware/clocks.h"
#include "hardware/pwm.h"
#include "steve.h"
#include "log.h"

#define SYS_CLOCK_FREQ
#define BUZZER_PIN 4
#define BUZZER_FREQ_HZ 1000
#define BUZZER_POWER 0.9 //Basically this is the duty cycle which also corresponds to amount of power

void buzzer_beep_job() {
  //Initialize pwm stuff
  logln_info("Beep!"); 
  gpio_set_function(BUZZER_PIN,  GPIO_FUNC_PWM);
  uint slice_num = pwm_gpio_to_slice_num(BUZZER_PIN);
  uint16_t wrap = (uint16_t) (clock_get_hz(clk_sys) / BUZZER_FREQ_HZ);
  pwm_set_wrap(slice_num, wrap);
  pwm_set_chan_level(slice_num, PWM_CHAN_A, (uint16_t)(BUZZER_POWER * wrap));

  for(int i = 0; i < 3; i++) {
    pwm_set_enabled(slice_num, true);
    vTaskDelay(pdMS_TO_TICKS(1000));
    pwm_set_enabled(slice_num, false);
    vTaskDelay(pdMS_TO_TICKS(1000));
  }
} 
