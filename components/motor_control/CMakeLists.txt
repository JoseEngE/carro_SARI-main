# Motor Control Component CMakeLists.txt

idf_component_register(
    SRCS "src/motor_control.c"
    INCLUDE_DIRS "include"
    PRIV_INCLUDE_DIRS "src"
    REQUIRES driver esp_timer freertos log
)
