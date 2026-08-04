#include "driver/gpio.h"

class Microphone {
	public: 
		Microphone(gpio_num_t workingIndicator, gpio_num_t serialDataPin, gpio_num_t clockPin, gpio_num_t leftRightPin, gpio_num_t wordSelectPin);
		void init();
		void start_record();
		void stop_record();

	private:
		gpio_num_t _workingIndicator;
		gpio_num_t _serialDataPin;
		gpio_num_t _clockPin;
		gpio_num_t _leftRightPin;
		gpio_num_t _wordSelectPin;

		void _toggle_indicator();
};