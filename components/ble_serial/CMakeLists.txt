idf_component_register(SRCS "src/ble_serial.c"
                       INCLUDE_DIRS "include"
                       REQUIRES bt nvs_flash log)
