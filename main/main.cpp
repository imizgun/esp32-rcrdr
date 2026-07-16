#include <cstdio>
#include <cstring>
#include <stdio.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "driver/gpio.h"
#include "dht11.hpp"
#include "lcd1602/lcd1602.h"
#include "driver/i2c_master.h"

#define DATA GPIO_NUM_19
#define LCD_I2C_ADDR 0x27

extern "C" void app_main(void)
{
    Dht11Reader reader(DATA);

    i2c_lowlevel_config config = {0, I2C_NUM_0, GPIO_NUM_22, GPIO_NUM_23};
    lcd1602_context lcd = lcd1602_init(LCD_I2C_ADDR, true, &config);
    if (!lcd) {
        printf("lcd init failed\n");
    }

    Dht11State lastState = {0, 0, false};

    while (1) {
        Dht11State state = reader.read();

        if (state.is_ok) {
            printf("temperature: %.1f, humidity: %.1f\n", state.temperature, state.humidity);
            if (lcd && state != lastState) {
                char line1[17];
                char line2[17];
                snprintf(line1, sizeof(line1), "T: %.1fC", state.temperature);
                snprintf(line2, sizeof(line2), "H: %.1f%%", state.humidity);

                lcd1602_clear(lcd);
                lcd1602_set_cursor(lcd, 0, 0);
                lcd1602_string(lcd, line1);
                lcd1602_set_cursor(lcd, 1, 0);
                lcd1602_string(lcd, line2);

                lastState = state;
            }
        } else {
            printf("read failed\n");
        }

        vTaskDelay(pdMS_TO_TICKS(2000));
    }
}
