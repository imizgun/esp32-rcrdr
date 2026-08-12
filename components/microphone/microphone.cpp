#include "microphone.hpp"
#include "driver/gpio.h"
#include "driver/i2s_common.h"
#include "driver/i2s_std.h"
#include "esp_intr_alloc.h"
#include "hal/gpio_types.h"
#include "freertos/FreeRTOS.h"
#include "esp_err.h"
#include <cstdio>

Microphone::Microphone(gpio_num_t workingIndicator, gpio_num_t serialDataPin, gpio_num_t clockPin, gpio_num_t leftRightPin, gpio_num_t wordSelectPin) {
	_workingIndicator = workingIndicator;
	_serialDataPin = serialDataPin;
	_clockPin = clockPin;
	_leftRightPin = leftRightPin;
	_wordSelectPin = wordSelectPin;
}

void Microphone::init() {
	gpio_reset_pin(this->_workingIndicator);
	gpio_set_direction(this->_workingIndicator, GPIO_MODE_INPUT_OUTPUT);

	gpio_reset_pin(this->_leftRightPin);
	gpio_set_direction(this->_leftRightPin, GPIO_MODE_OUTPUT);
	gpio_set_level(this->_leftRightPin, 0);

    this->_rx_chan = NULL;
    i2s_chan_config_t chan_cfg = I2S_CHANNEL_DEFAULT_CONFIG(I2S_NUM_0, I2S_ROLE_MASTER);
    esp_err_t err = i2s_new_channel(&chan_cfg, NULL, &this->_rx_chan);
    printf("i2s_new_channel: %s\n", esp_err_to_name(err));

    i2s_std_config_t std_cfg = {
        .clk_cfg = I2S_STD_CLK_DEFAULT_CONFIG(44100),
        .slot_cfg = I2S_STD_MSB_SLOT_DEFAULT_CONFIG(I2S_DATA_BIT_WIDTH_16BIT, I2S_SLOT_MODE_MONO),
        .gpio_cfg = {
            .mclk = I2S_GPIO_UNUSED,
            .bclk = this->_clockPin,
            .ws = this->_wordSelectPin,
            .dout = I2S_GPIO_UNUSED,
            .din = this->_serialDataPin,
            .invert_flags = {},
        }
    };

    err = i2s_channel_init_std_mode(this->_rx_chan, &std_cfg);
    printf("i2s_channel_init_std_mode: %s\n", esp_err_to_name(err));
}

void Microphone::start_record() {
	_toggle_indicator();
	i2s_channel_enable(this->_rx_chan);
	_recording = true;
	
	xTaskCreate(&Microphone::readTaskEntry, "miс:read", 4096, this, 5, &_read_task_handle);
}

void Microphone::stop_record() {
	_toggle_indicator();
	_recording = false;
	i2s_channel_disable(this->_rx_chan);
}

size_t Microphone::read(int16_t* buffer, size_t max_samples) {
	size_t bytes_read = 0;
	i2s_channel_read(this->_rx_chan, buffer, max_samples * sizeof(int16_t), &bytes_read, pdMS_TO_TICKS(100));
	return bytes_read / sizeof(int16_t);
}

void Microphone::_toggle_indicator() {
	int level = gpio_get_level(this->_workingIndicator);
	gpio_set_level(this->_workingIndicator, !level);
}

void Microphone::readTaskEntry(void* arg) {
	static_cast<Microphone*>(arg)->_readLoop();
}

void Microphone::_readLoop() {
	int16_t buffer[256];
	while (_recording) {
		size_t res = this->read(buffer, 256);

		printf("read: %zu/%d\n", res, 256);
	}
	vTaskDelete(NULL);
}