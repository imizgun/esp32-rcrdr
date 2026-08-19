#include <cstdio>
#include <cstring>
#include <stdio.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "driver/gpio.h"
#include "driver/i2c_master.h"
#include "recorder.hpp"
#include "soc/gpio_num.h"

#define MIC_INDC_PIN GPIO_NUM_26
#define RECORD_BUTTON_PIN GPIO_NUM_25

#define SD_CS_PIN GPIO_NUM_27
#define SD_CLK_PIN GPIO_NUM_14
#define SD_MOSI_PIN GPIO_NUM_13
#define SD_MISO_PIN GPIO_NUM_22

extern "C" void app_main(void)
{
	Microphone mic = Microphone(MIC_INDC_PIN,
                          		GPIO_NUM_18,
                          		GPIO_NUM_16,
                          		GPIO_NUM_4,
                          		GPIO_NUM_17);
	mic.init();

	Memory memory = Memory(SD_CS_PIN, SD_CLK_PIN, SD_MOSI_PIN, SD_MISO_PIN);

	Recorder rec = Recorder(&mic, &memory, RECORD_BUTTON_PIN);
	rec.init();
    
    while (1) {
    	vTaskDelay(pdMS_TO_TICKS(1000));
    }
}
