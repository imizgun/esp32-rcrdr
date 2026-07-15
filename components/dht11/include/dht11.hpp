#pragma once

#include "driver/gpio.h"
#include "soc/gpio_num.h"

struct Dht11State {
    float temperature;
    float humidity;
    bool is_ok;
};

class Dht11Reader {
    public:
        Dht11Reader(gpio_num_t pin);
        Dht11State read();

    private:
        Dht11State _state;
        gpio_num_t _pin;
};
