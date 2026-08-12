#include "driver/gpio.h"
#include "hal/gpio_types.h"
#include "esp_err.h"
#include <cstdio>
#include "recorder.hpp"
#include "driver/i2s_std.h"

Recorder::Recorder(Microphone* microphone, gpio_num_t button_pin) {
	this->_microphone = microphone;
	this->_button_pin = button_pin;
	this->_isRecording = false;
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
	_microphone->start_record();
	_isRecording = true;
}

void Recorder::stopRecording() {
	_microphone->stop_record();
	_isRecording = false;
}
