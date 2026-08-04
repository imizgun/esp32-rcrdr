#include "microphone.hpp"
#include "driver/gpio.h"
#include "esp_intr_alloc.h"
#include "hal/gpio_types.h"

Microphone::Microphone(gpio_num_t workingIndicator, gpio_num_t serialDataPin, gpio_num_t clockPin, gpio_num_t leftRightPin, gpio_num_t wordSelectPin) {
	_workingIndicator = workingIndicator;
	_serialDataPin = serialDataPin;
	_clockPin = clockPin;
	_leftRightPin = leftRightPin;
	_wordSelectPin = wordSelectPin;
}

void Microphone::init() {
	gpio_reset_pin(this->_workingIndicator);
	gpio_set_direction(this->_workingIndicator, GPIO_MODE_OUTPUT);
	_toggle_indicator();
}

void Microphone::start_record() {
	gpio_reset_pin(this->_workingIndicator);
	_toggle_indicator();
}

void Microphone::stop_record() {
	_toggle_indicator();
}

void Microphone::_toggle_indicator() {
	int level = gpio_get_level(this->_workingIndicator);
	gpio_set_level(this->_workingIndicator, !level);
}