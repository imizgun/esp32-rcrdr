#include <stdio.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "driver/gpio.h"
#include "dht11.hpp"

#define DATA GPIO_NUM_19

// #define RED GPIO_NUM_17
// #define YELLOW GPIO_NUM_5
// #define GREEN GPIO_NUM_18

extern "C" void app_main(void)
{
    // TrafficLight light = TrafficLight();
    
    // gpio_reset_pin(RED);
    // gpio_set_direction(RED, GPIO_MODE_OUTPUT);
    // gpio_reset_pin(YELLOW);
    // gpio_set_direction(YELLOW, GPIO_MODE_OUTPUT);
    // gpio_reset_pin(GREEN);
    // gpio_set_direction(GREEN, GPIO_MODE_OUTPUT);

    // while (1) {
    //     switch (light.getState()) {
    //         case TrafficLightState::ChangingState:
    //             gpio_set_level(RED, 1);
    //             gpio_set_level(YELLOW, 1);
    //             break;
    //         case TrafficLightState::Allowed:
    //             gpio_set_level(RED, 0);
    //             gpio_set_level(YELLOW, 0);
    //             gpio_set_level(GREEN, 1);
    //             break;
    //         case TrafficLightState::BeReadyToStop:        
    //             gpio_set_level(YELLOW, 1);
    //             gpio_set_level(GREEN, 0);
    //             break;
    //         case TrafficLightState::Forbidden:
    //             gpio_set_level(RED, 1);
    //             gpio_set_level(GREEN, 0);
    //             gpio_set_level(YELLOW, 0);
    //             break;
    //     }
    //     vTaskDelay(light.getStateTime() / portTICK_PERIOD_MS);
    //     light.update();
    // }
    // 

    Dht11Reader reader(DATA);

    while (1) {
        Dht11State state = reader.read();

        if (state.is_ok) {
            printf("temperature: %.1f, humidity: %.1f\n", state.temperature, state.humidity);
        } else {
            printf("read failed\n");
        }

        vTaskDelay(pdMS_TO_TICKS(2000));
    }
}
