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

extern "C" void app_main(void)
{
	Microphone mic = Microphone(MIC_INDC_PIN, 
                          		GPIO_NUM_18, 
                          		GPIO_NUM_16, 
                          		GPIO_NUM_4, 
                          		GPIO_NUM_17);
	mic.init();

	Recorder rec = Recorder(&mic, RECORD_BUTTON_PIN);
	rec.init();
    
    while (1) {
    	vTaskDelay(pdMS_TO_TICKS(1000));
    }
}
