#pragma once

#include "driver/ledc.h"
#include "esp_err.h"

#ifdef __cplusplus
extern "C" {
#endif

/**
 * @brief Configuration structure for the servo
 */
typedef struct {
    int gpio_num;               /*!< GPIO number to output PWM signal */
    ledc_timer_t timer_number;  /*!< LEDC timer number */
    ledc_channel_t channel_number; /*!< LEDC channel number */
    int min_pulse_width_us;     /*!< Minimum pulse width in microseconds (0 degrees) */
    int max_pulse_width_us;     /*!< Maximum pulse width in microseconds (180 degrees) */
    int frequency;              /*!< PWM frequency in Hz (usually 50Hz) */
    float min_angle;            /*!< Minimum allowable angle */
    float max_angle;            /*!< Maximum allowable angle */
    float initial_angle;        /*!< Initial angle on startup */
} servo_config_t;

/**
 * @brief Initialize the servo control using LEDC
 * 
 * @param config Pointer to the configuration structure
 * @return esp_err_t ESP_OK on success
 */
esp_err_t servo_init(const servo_config_t *config);

/**
 * @brief Set the servo angle
 * 
 * @param angle Angle in degrees (0.0 to 180.0)
 * @return esp_err_t ESP_OK on success
 */
esp_err_t servo_set_angle(float angle);

#ifdef __cplusplus
}
#endif
