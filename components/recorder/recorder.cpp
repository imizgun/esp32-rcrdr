#include "driver/gpio.h"
#include "hal/gpio_types.h"
#include "recorder.hpp"

Recorder::Recorder(Microphone* microphone, gpio_num_t button_pin) {
	this->_microphone = microphone;
	this->_button_pin = button_pin;
	this->_isRecording = false;
	this->_task_handle = nullptr;
	this->_last_isr_time_us = 0;
}

void Recorder::init() {
	gpio_config_t io_conf = {
        .pin_bit_mask = (1ULL << _button_pin),
        .mode = GPIO_MODE_INPUT,
        .pull_up_en = GPIO_PULLUP_ENABLE,
        .pull_down_en = GPIO_PULLDOWN_DISABLE,
        .intr_type = GPIO_INTR_NEGEDGE
    };
    gpio_config(&io_conf);

    xTaskCreate(&Recorder::taskEntry, "recorder_task", 4096, this, 10, &_task_handle);

    gpio_install_isr_service(0);
    gpio_isr_handler_add(_button_pin, &Recorder::isr_handler, this);
}

void Recorder::taskEntry(void* arg) {
	static_cast<Recorder*>(arg)->run();
}

void Recorder::run() {
	while (true) {
		ulTaskNotifyTake(pdTRUE, portMAX_DELAY);
		onInterrupt();
	}
}

void Recorder::onInterrupt() {
	if (_isRecording) {
		stopRecording();
	} else {
		startRecording();
	}
}

void Recorder::startRecording() {
	_microphone->start_record();
	_isRecording = true;
}

void Recorder::stopRecording() {
	_microphone->stop_record();
	_isRecording = false;
}
