#include "microphone.hpp"
#include "soc/gpio_num.h"
#include "esp_attr.h"
#include "esp_timer.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"

#define DEBOUNCE_US 200000

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
		volatile int64_t _last_isr_time_us;

		static void IRAM_ATTR isr_handler(void* arg) {
			Recorder* self = static_cast<Recorder*>(arg);

			int64_t now = esp_timer_get_time();
			if (now - self->_last_isr_time_us < DEBOUNCE_US) {
				return;
			}
			self->_last_isr_time_us = now;

			BaseType_t higher_priority_task_woken = pdFALSE;
			vTaskNotifyGiveFromISR(self->_task_handle, &higher_priority_task_woken);
			portYIELD_FROM_ISR(higher_priority_task_woken);
		};

		static void taskEntry(void* arg);
		void run();
		void onInterrupt();
};