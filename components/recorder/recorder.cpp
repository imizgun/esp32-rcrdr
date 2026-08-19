#include "driver/gpio.h"
#include "hal/gpio_types.h"
#include "esp_err.h"
#include <cstdio>
#include <cstdlib>
#include <cstring>
#include "recorder.hpp"
#include "driver/i2s_std.h"

// must match the I2S config set up in Microphone::init()
#define RECORDING_SAMPLE_RATE 44100
#define RECORDING_BITS_PER_SAMPLE 16
#define RECORDING_CHANNELS 1

Recorder::Recorder(Microphone* microphone, Memory* memory, gpio_num_t button_pin) {
	this->_microphone = microphone;
	this->_memory = memory;
	this->_button_pin = button_pin;
	this->_isRecording = false;
	this->_recording_count = 0;
	this->_task_handle = nullptr;
}

void Recorder::init() {
	gpio_config_t io_conf = {
        .pin_bit_mask = (1ULL << _button_pin),
        .mode = GPIO_MODE_INPUT,
        .pull_up_en = GPIO_PULLUP_ENABLE,
        .pull_down_en = GPIO_PULLDOWN_DISABLE,
        .intr_type = GPIO_INTR_NEGEDGE
    };
    esp_err_t err = gpio_config(&io_conf);
    printf("gpio_config: %s\n", esp_err_to_name(err));

    xTaskCreate(&Recorder::taskEntry, "recorder_task", 4096, this, 10, &_task_handle);

    err = gpio_install_isr_service(0);
    printf("gpio_install_isr_service: %s\n", esp_err_to_name(err));
    err = gpio_isr_handler_add(_button_pin, &Recorder::isr_handler, this);
    printf("gpio_isr_handler_add: %s\n", esp_err_to_name(err));
}

void IRAM_ATTR Recorder::isr_handler(void* arg) {
	Recorder* self = static_cast<Recorder*>(arg);

	BaseType_t higher_priority_task_woken = pdFALSE;
	vTaskNotifyGiveFromISR(self->_task_handle, &higher_priority_task_woken);
	portYIELD_FROM_ISR(higher_priority_task_woken);
}

void Recorder::taskEntry(void* arg) {
	static_cast<Recorder*>(arg)->run();
}

void Recorder::run() {
	while (true) {
		ulTaskNotifyTake(pdTRUE, portMAX_DELAY);
		vTaskDelay(pdMS_TO_TICKS(DEBOUNCE_SETTLE_MS));
		if (gpio_get_level(_button_pin) == 0) {
			onInterrupt();
		}
	}
}

void Recorder::onInterrupt() {
	printf("interrupt\n");
	if (_isRecording) {
		stopRecording();
	} else {
		startRecording();
	}
}

void Recorder::startRecording() {
    char rec_count_c[3];
    char record[] = "record_";
    char* fileName = strcat(strcat(record, itoa(this->_recording_count, rec_count_c, 10)), ".wav");
    
	// TODO(you): _memory->startFile(filename, RECORDING_SAMPLE_RATE,
	// RECORDING_BITS_PER_SAMPLE, RECORDING_CHANNELS) — параметры должны совпадать
	// с тем, что реально настроено в Microphone::init() (I2S_STD_CLK_DEFAULT_CONFIG
	// и слот-конфиг). Провалидировать результат перед тем как продолжать.

	_microphone->start_record();
	_isRecording = true;
	_memory->startFile(fileName, 44100, 16, 1);

	xTaskCreate(&Recorder::pumpTaskEntry, "recorder_pump", 4096, this, 5, &_pump_task_handle);
}

void Recorder::stopRecording() {
	// Просто сигнализируем насосу остановиться — сам файл закрывает pumpLoop(),
	// чтобы не было гонки между fclose() отсюда и ещё не дописанными данными там.
	_isRecording = false;
	_microphone->stop_record();
}

void Recorder::pumpTaskEntry(void* arg) {
	static_cast<Recorder*>(arg)->pumpLoop();
}

// TODO(you): основной алгоритм передачи звука на SD.
//   1. Завести локальный буфер int16_t buffer[N] на стеке. Выбор N — компромисс
//      между тем, как часто дёргаем fwrite (см. Memory::writeSamples) и
//      размером стека задачи (сейчас стек 4096 байт в xTaskCreate выше).
//   2. Пока _isRecording (или пока _microphone->is_recording(), подумай какое
//      условие правильнее) — читать: size_t n = _microphone->read(buffer, N);
//      если n > 0, писать: _memory->writeSamples(buffer, n).
//   3. После выхода из цикла — доп. чтение "на добор" остатка из буфера
//      микрофона (иначе последние сэмплы, прочитанные _readLoop до остановки,
//      потеряются), затем _memory->stopFile().
//   4. vTaskDelete(NULL) в конце, обнулить _pump_task_handle.
void Recorder::pumpLoop() {
}
