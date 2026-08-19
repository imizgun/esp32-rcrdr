#pragma once

#include "driver/gpio.h"
#include "driver/i2s_types.h"
#include "freertos/idf_additions.h"

class Microphone {
	public:
		Microphone(gpio_num_t workingIndicator, gpio_num_t serialDataPin, gpio_num_t clockPin,
		gpio_num_t leftRightPin, gpio_num_t wordSelectPin);
		void init();
		void start_record();
		void stop_record();
		// Забирает то, что успел накопить _readLoop в read_data. Это единственная
		// точка доступа к аудиопотоку снаружи класса.
		size_t read(int16_t* buffer, size_t max_samples);
		bool is_recording();

	private:
		volatile bool _recording;
		TaskHandle_t _read_task_handle;
		gpio_num_t _workingIndicator;
		gpio_num_t _serialDataPin;
		gpio_num_t _clockPin;
		gpio_num_t _leftRightPin;
		gpio_num_t _wordSelectPin;
		i2s_chan_handle_t _rx_chan;
		// Единственный буфер между продюсером (_readLoop, читает I2S) и
		// потребителем (read(), вызывается снаружи). Размер задан в init() —
		// TODO(you): пересчитать под реальный размер чанка/частоту записи на SD.
		StreamBufferHandle_t read_data;

		void _toggle_indicator();
		static void readTaskEntry(void* arg);
		void _readLoop();
		// Прямое чтение из I2S. Вызывать может только _readLoop — это и есть
		// тот самый единственный читатель хардварного канала.
		size_t _readFromI2S(int16_t* buffer, size_t max_samples);
};