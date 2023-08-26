#include <RadioLib.h>
#include <stdio.h>
#include "pico/stdlib.h"
#include "hardware/adc.h"
#include "hardware/spi.h"

class PiHal : public RadioLibHal {
    public:
        // speed taken from radio lib pi example
        PiHal(uint32_t spiSpeed = 2000000)
            : RadioLibHal(0, 1, 0, 1, 0, 1), // PI_INPUT, PI_OUTPUT, PI_LOW, PI_HIGH, RISING_EDGE, FALLING_EDGE - taken from pigpio.h, I am assuming it will be the same
            _spiSpeed(spiSpeed) {
        }

        void init() override {
            // Enable SPI using the function defined later in this class
            spiBegin();

            // anything else we need to do to initialize the radio?
        }

        // stop the spi
        void term() override {
            // Disable spi using function defined later in this class
            spiEnd();
        }

        // Mode: 0 for in, 1 for out
        // GPIO-related methods (pinMode, digitalWrite etc.) should check
        // RADIOLIB_NC as an alias for non-connected pins
        void pinMode(uint32_t pin, uint32_t mode) override {
            if(pin == RADIOLIB_NC) {
                return;
            }

            gpio_init(pin);
            gpio_set_dir(pin, mode);
        }

        void digitalWrite(uint32_t pin, uint32_t value) override {
            if(pin == RADIOLIB_NC) {
                return;
            }

            gpio_put(pin, value);
        }

        uint32_t digitalRead(uint32_t pin) override {
            if (pin == RADIOLIB_NC) {
                return(0);
            }

            return(gpio_get(pin));
        }

        void attachInterrupt(uint32_t interruptNum, void (*interruptCb)(void), uint32_t mode) override {
            if(interruptNum == RADIOLIB_NC) {
                return;
            }

            // interruptNum is being input as the GPIO, will this still work?
            // mode is either GPIO_IRQ_EDGE_FALL or GPIO_IRQ_EDGE_RISE            
            gpio_set_irq_enabled_with_callback(interruptNum, mode, true, (gpio_irq_callback_t)interruptCb);
        }

        void detachInterrupt(uint32_t interruptNum) override {
            if(interruptNum == RADIOLIB_NC) {
                return;
            }

            gpio_set_irq_enabled_with_callback(interruptNum, 0, false, NULL);
        }

        void delay(unsigned long ms) override {
            sleep_ms(ms);
        }

        void delayMicroseconds(unsigned long us) override {
            sleep_ms(us / 1000);
        }

        unsigned long millis() override {
            // time_us_64 returns timestamp in microseconds
            return(time_us_64() / 1000);
        }

        unsigned long micros() override {
            return(time_us_64());
        }

        
        long pulseIn(uint32_t pin, uint32_t state, unsigned long timeout) override {
            if(pin == RADIOLIB_NC) {
                return(0);
            }

            gpio_init(pin);
            gpio_set_dir(pin, 0); // 0 for input
            
            uint32_t start = time_us_64();

            while(gpio_get(pin) == state) {
                if((time_us_64() - start) > timeout) {
                    return(0);
                }
            }
            // returns in microseconds
            return(time_us_64() - start);
        }

        void spiBegin() {
            if(_spiHandle < 0) {
                _spiHandle = spi_init (spi_default, _spiSpeed);
            }
        }

        void spiBeginTransaction() {}

        uint8_t spiTransfer(uint8_t b) {
            char ret;
            const uint8_t* outputBuffer = &b;
            spi_write_blocking(spi_default, outputBuffer, 1);
            
            return(b);
        }

        void spiEndTransaction() {}

        void spiEnd() {
            if(_spiHandle >= 0) {
                spi_deinit(spi_default);
                _spiHandle = -1;
            }
        }


    private:
        const unsigned int _spiSpeed;
        //const uint8_t _spiChannel; - for now just use spi0
        int _spiHandle = -1;
};