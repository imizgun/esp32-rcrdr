#include "microphone.hpp"
#include "soc/gpio_num.h"
#include "esp_attr.h"
#include "esp_timer.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"

#define DEBOUNCE_SETTLE_MS 30

class Recorder {
	public:
		Recorder(Microphone*  microphone, gpio_num_t button_pin);
		void init();
		void startRecording();
		void stopRecording();

	private:
		Microphone* _microphone;
		gpio_num_t _button_pin;
		bool _isRecording;
		TaskHandle_t _task_handle;

		static void IRAM_ATTR isr_handler(void* arg);

		static void taskEntry(void* arg);
		void run();
		void onInterrupt();
};