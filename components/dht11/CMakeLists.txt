idf_component_register(SRCS "dht11.cpp"
                        INCLUDE_DIRS "include"
                        PRIV_REQUIRES esp_driver_gpio
                        PRIV_REQUIRES esp_timer)
