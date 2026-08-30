#include "driver/gpio.h"
#include "freertos/idf_additions.h"
#include "hal/gpio_types.h"
#include "esp_err.h"
#include <cstdio>
#include <cstdlib>
#include <cstring>
#include "recorder.hpp"
#include "driver/i2s_std.h"

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
    char fileName[32];
    snprintf(fileName, sizeof(fileName), "record_%d.wav", this->_recording_count);

	_microphone->start_record();
	_isRecording = true;
	bool started = _memory->startFile(fileName, RECORDING_SAMPLE_RATE, RECORDING_BITS_PER_SAMPLE, RECORDING_CHANNELS);

	if (!started) {
	    printf("Bad file writing start\n");
		return;
	}

	xTaskCreate(&Recorder::pumpTaskEntry, "recorder_pump", 4096, this, 5, &_pump_task_handle);
}

void Recorder::stopRecording() {
	_isRecording = false;
	_microphone->stop_record();
}

void Recorder::pumpTaskEntry(void* arg) {
	static_cast<Recorder*>(arg)->pumpLoop();
}

void Recorder::pumpLoop() {
    this->_recording_count++;
    
    const size_t N = 256;
    int16_t buffer[N];
    
    while (_isRecording) {
        bool res = pump(buffer, N);

        if (!res) return;
    }
    
    pump(buffer, N);
    _memory->stopFile();
    
    _pump_task_handle = nullptr;
    vTaskDelete(NULL);
}


bool Recorder::pump(int16_t* buffer, size_t n) { 
    size_t read = _microphone->read(buffer, n);

    if (read > 0) {
        _memory->writeSamples(buffer, read);
        if (_memory->isWritingFailed()) {
            _isRecording = false;
            _memory->stopFile();
            _pump_task_handle = nullptr;
            vTaskDelete(NULL);
            return false;
        }
    }
    return true;
}