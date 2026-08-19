#pragma once

#include "microphone.hpp"
#include "memory.hpp"
#include "soc/gpio_num.h"
#include "esp_attr.h"
#include "esp_timer.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "driver/i2s_std.h"

#define DEBOUNCE_SETTLE_MS 30

class Recorder {
	public:
		Recorder(Microphone* microphone, Memory* memory, gpio_num_t button_pin);
		void init();
		void startRecording();
		void stopRecording();

	private:
		Microphone* _microphone;
		Memory* _memory;
		gpio_num_t _button_pin;
		bool _isRecording;
		int _recording_count;
		TaskHandle_t _task_handle;
		// Задача-насос: читает из _microphone->read(), пишет в _memory->writeSamples().
		TaskHandle_t _pump_task_handle;

		static void IRAM_ATTR isr_handler(void* arg);

		static void taskEntry(void* arg);
		void run();
		void onInterrupt();

		static void pumpTaskEntry(void* arg);
		void pumpLoop();
};