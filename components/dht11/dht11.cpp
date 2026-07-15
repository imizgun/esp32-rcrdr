#include "dht11.hpp"
#include "esp_system.h"
#include "freertos/idf_additions.h"
#include "freertos/projdefs.h"
#include "hal/gpio_types.h"
#include "driver/gpio.h"
#include "soc/gpio_num.h"
#include "soc/interrupts.h"
#include "esp_timer.h"
#include "esp_rom_sys.h"
#include <cstdint>

static bool waitForHandshake(gpio_num_t pin, int level, int64_t timeout) {
    int64_t start = esp_timer_get_time();

    while (gpio_get_level(pin) != level) {
        if (esp_timer_get_time() - start > timeout) return false; 
    }

    return true;
}

Dht11Reader::Dht11Reader(gpio_num_t pin) {
    _pin = pin;
}

Dht11State Dht11Reader::read() {
    // sending signal to sensor
    Dht11State state = {0, 0, false};


    gpio_reset_pin(_pin);
    gpio_set_direction(_pin, GPIO_MODE_OUTPUT);
    gpio_set_level(_pin, 0);
    vTaskDelay(pdMS_TO_TICKS(20));
    gpio_set_level(_pin, 1);
    esp_rom_delay_us(30);
    gpio_set_direction(_pin, GPIO_MODE_INPUT);

    if (!waitForHandshake(_pin, 0, 100)) { printf("fail: stage 1 (initial low)\n"); return state; }
    if (!waitForHandshake(_pin, 1, 100)) { printf("fail: stage 2 (response high)\n"); return state; }
    if (!waitForHandshake(_pin, 0, 100)) { printf("fail: stage 3 (bit0 low)\n"); return state; }

    uint8_t data[5] = {0};

    for (int i = 0; i < 40; i++) {
        if (!waitForHandshake(_pin, 1, 100)) { printf("fail: bit %d rising edge\n", i); return state; }
        uint64_t t0 = esp_timer_get_time();
        if (!waitForHandshake(_pin, 0, 100)) { printf("fail: bit %d falling edge\n", i); return state; }
        bool bit = esp_timer_get_time() - t0 > 40;

        data[i / 8] <<= 1;
        if (bit) data[i / 8] |= 1;
    }

    printf("raw: %02x %02x %02x %02x %02x\n", data[0], data[1], data[2], data[3], data[4]);

    state.is_ok = data[4] == (uint8_t)(data[0] + data[1] + data[2] + data[3]);

    if (state.is_ok) {
        state.humidity = (float)data[0] + (float)data[1] / 100;
        state.temperature = (float)data[2] + (float)data[3] / 100;
    }

    return state;
}
