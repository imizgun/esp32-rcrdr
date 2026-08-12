idf_component_register(SRCS "microphone.cpp"
		INCLUDE_DIRS "include"
		REQUIRES esp_driver_i2s
		PRIV_REQUIRES esp_driver_gpio)