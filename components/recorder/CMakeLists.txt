idf_component_register(SRCS "recorder.cpp"
		INCLUDE_DIRS "include"
		REQUIRES microphone
		PRIV_REQUIRES esp_driver_gpio esp_timer)