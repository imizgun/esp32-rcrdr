#include <stdio.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "driver/gpio.h"


#define LED GPIO_NUM_2

extern "C" void app_main(void)
{    
    gpio_reset_pin(LED);
    gpio_set_direction(LED, GPIO_MODE_OUTPUT);

    while (1) {
        printf("LED ON\n");
        gpio_set_level(LED, 1);
        vTaskDelay(1000 / portTICK_PERIOD_MS); // Delay 1 second

        printf("LED OFF\n");
        gpio_set_level(LED, 0);
        vTaskDelay(1000 / portTICK_PERIOD_MS); // Delay 1 second
    }
}
