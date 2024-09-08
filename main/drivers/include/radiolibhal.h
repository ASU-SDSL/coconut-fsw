#pragma once

#include "pico/stdlib.h"
#include "hardware/adc.h"
#include "hardware/spi.h"
#include "radio.h"
#include "RadioLib.h"
//#include <SX1278.h>

class PiPicoHal : public RadioLibHal { 
public:
    PiPicoHal(spi_inst_t *spi, uint32_t spi_speed = 2000000)
        : RadioLibHal(0, 1, 0, 1, 0, 1),  
          _spi(spi),
          _spi_speed(spi_speed) {
            printf("PiPicoHal Constructor\n");
          }

    void init() override {
        stdio_init_all();
        spiBegin();

    }

    // used in deconstructor 
    void term() override {
        spiEnd();
    }

    void pinMode(uint32_t pin, uint32_t mode) override {
        if (pin == RADIOLIB_NC) {
            return;
        }
        gpio_init(pin);
        gpio_set_dir(pin, mode);
    }

    void digitalWrite(uint32_t pin, uint32_t value) override {
        if (pin == RADIOLIB_NC) {
            return;
        }
        gpio_put(pin, value);
    }

    uint32_t digitalRead(uint32_t pin) override {
        if (pin == RADIOLIB_NC) {
            return 0;
        }
        return gpio_get(pin);
    }

    typedef void(* gpio_irq_callback_t) (uint gpio, uint32_t event_mask);
    //void gpio_set_irq_enabled_with_callback (uint gpio, uint32_t event_mask, bool enabled, gpio_irq_callback_t callback);
    void attachInterrupt(uint32_t interruptNum, void (*interruptCb)(void), uint32_t mode) override {
        if (interruptNum == RADIOLIB_NC) {
            return;
        }

        // Setting the interrupt with the Raspberry Pi Pico SDK
        gpio_set_irq_enabled_with_callback(interruptNum, mode, true, (gpio_irq_callback_t)interruptCb);
    }

    void detachInterrupt(uint32_t interruptNum) override {
        if (interruptNum == RADIOLIB_NC) {
            return;
        }

        // Disabling the interrupt with the Raspberry Pi Pico SDK
        gpio_set_irq_enabled_with_callback(interruptNum, 0, false, NULL);
    }

    void delay(unsigned long ms) override {
        sleep_ms(ms);
    }

    void delayMicroseconds(unsigned long us) override {
        sleep_us(us / 1000);
    }

    unsigned long millis() override {
        return to_ms_since_boot(get_absolute_time());
    }

    unsigned long micros() override {
        return to_us_since_boot(get_absolute_time()) / 1000;
    }

    // suspect
    long pulseIn(uint32_t pin, uint32_t state, unsigned long timeout) override {
        printf("pulseIn\n");
        if (pin == RADIOLIB_NC) {
            return 0;
        }

        gpio_set_dir(pin, GPIO_IN);
        uint32_t start = this->micros();
        uint32_t curtick = this->micros();
        uint32_t timeoutMicros = timeout; // * 1000; 

        while (gpio_get(pin) == state) {
            if ((this->micros() - curtick) > timeoutMicros) {
                return 0;
            }
        }

        return (this->micros() - start);
    }

    void spiBegin() override {
        printf("spiBegin\n");
        spi_init(_spi, _spi_speed);
        spi_set_format(_spi, 8, SPI_CPOL_0, SPI_CPHA_0, SPI_MSB_FIRST);
    }

    void spiBeginTransaction() override {
    }

    // suspect
    void spiTransfer(uint8_t *out, size_t len, uint8_t *in) override {
        spi_write_read_blocking(_spi, in, out, len);
    }

    void spiEndTransaction() override {
    }

    void spiEnd() override {
        spi_deinit(_spi);
    }

private:
    const uint32_t _spi_speed;
    spi_inst_t *_spi;
    int _spiHandle = -1;
};
