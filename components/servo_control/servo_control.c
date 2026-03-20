#include "servo_control.h"
#include "esp_log.h"
#include <math.h>

static const char *TAG = "servo_control";
static servo_config_t *driver_config = NULL;
static uint32_t duty_resolution_max_value = 0; // Calculated based on resolution

esp_err_t servo_init(const servo_config_t *config)
{
    if (config == NULL) {
        return ESP_ERR_INVALID_ARG;
    }

    // Allocate memory to save the config (simple implementation, one instance support for now)
    if (driver_config == NULL) {
        driver_config = malloc(sizeof(servo_config_t));
        if (driver_config == NULL) {
            return ESP_ERR_NO_MEM;
        }
    }
    *driver_config = *config;

    // Prepare and then apply the LEDC PWM timer configuration
    ledc_timer_config_t ledc_timer = {
        .speed_mode       = LEDC_LOW_SPEED_MODE,
        .timer_num        = config->timer_number,
        .duty_resolution  = LEDC_TIMER_13_BIT,
        .freq_hz          = config->frequency,  // Set output frequency at 50Hz
        .clk_cfg          = LEDC_AUTO_CLK
    };
    ESP_ERROR_CHECK(ledc_timer_config(&ledc_timer));

    // Prepare and then apply the LEDC PWM channel configuration
    ledc_channel_config_t ledc_channel = {
        .speed_mode     = LEDC_LOW_SPEED_MODE,
        .channel        = config->channel_number,
        .timer_sel      = config->timer_number,
        .intr_type      = LEDC_INTR_DISABLE,
        .gpio_num       = config->gpio_num,
        .duty           = 0, // Set duty to 0%
        .hpoint         = 0
    };
    ESP_ERROR_CHECK(ledc_channel_config(&ledc_channel));

    // Calculate max duty resolution value (2^13 - 1)
    duty_resolution_max_value = (1 << LEDC_TIMER_13_BIT) - 1;

    ESP_LOGI(TAG, "Servo initialized on GPIO %d", config->gpio_num);
    
    // Set initial angle
    servo_set_angle(config->initial_angle);
    
    return ESP_OK;
}

esp_err_t servo_set_angle(float angle)
{
    if (driver_config == NULL) {
        ESP_LOGE(TAG, "Servo not initialized");
        return ESP_ERR_INVALID_STATE;
    }

    if (angle < driver_config->min_angle) angle = driver_config->min_angle;
    if (angle > driver_config->max_angle) angle = driver_config->max_angle;

    // Map angle to pulse width
    // Linear mapping: pulse_width = min + (angle / 180.0) * (max - min)
    float pulse_width_us = driver_config->min_pulse_width_us + 
                           (angle / 180.0f) * (driver_config->max_pulse_width_us - driver_config->min_pulse_width_us);

    // Calculate duty cycle:
    // Duty = (pulse_width_us / period_us) * max_resolution_value
    // period_us = 1000000 / frequency
    float period_us = 1000000.0f / driver_config->frequency;
    uint32_t duty = (uint32_t)((pulse_width_us / period_us) * duty_resolution_max_value);

    ESP_ERROR_CHECK(ledc_set_duty(LEDC_LOW_SPEED_MODE, driver_config->channel_number, duty));
    ESP_ERROR_CHECK(ledc_update_duty(LEDC_LOW_SPEED_MODE, driver_config->channel_number));

    return ESP_OK;
}
