/**
 * @file motor_control.h
 * @brief Motor Control Component for RC Car with Ackermann Steering
 * 
 * Controls two DC motors using MX1508 drivers:
 * - Rear motor: Drive (forward/backward)
 * - Front motor: Steering (left/right)
 */

#ifndef MOTOR_CONTROL_H
#define MOTOR_CONTROL_H

#include <stdint.h>
#include <stdbool.h>
#include "esp_err.h"
#include "driver/gpio.h"
#include "driver/ledc.h"

#ifdef __cplusplus
extern "C" {
#endif

/**
 * @brief Motor identifier
 */
typedef enum {
    MOTOR_DRIVE,      /*!< Rear motor (drive/traction) */
    MOTOR_STEERING    /*!< Front motor (steering/direction) */
} motor_id_t;

/**
 * @brief Steering position presets
 */
typedef enum {
    STEER_LEFT,       /*!< Turn left */
    STEER_CENTER,     /*!< Straight ahead */
    STEER_RIGHT       /*!< Turn right */
} steering_position_t;

/**
 * @brief Motor configuration structure
 */
typedef struct {
    gpio_num_t in1_pin;           /*!< IN1 pin of MX1508 */
    gpio_num_t in2_pin;           /*!< IN2 pin of MX1508 */
    uint32_t pwm_freq_hz;         /*!< PWM frequency in Hz (default: 1000) */
    ledc_timer_t timer;           /*!< LEDC timer to use */
    ledc_channel_t channel_a;     /*!< LEDC channel for IN1 */
    ledc_channel_t channel_b;     /*!< LEDC channel for IN2 */
} motor_config_t;

/**
 * @brief Initialize motor control system
 * 
 * @param drive_config Configuration for drive motor (rear)
 * @param steering_config Configuration for steering motor (front)
 * @return ESP_OK on success
 */
esp_err_t motor_control_init(const motor_config_t* drive_config, 
                              const motor_config_t* steering_config);

/**
 * @brief Set drive motor speed
 * 
 * @param speed Speed from -100 (full backward) to +100 (full forward), 0 = stop
 * @return ESP_OK on success
 */
esp_err_t motor_drive_set_speed(int8_t speed);

/**
 * @brief Move forward at specified speed
 * 
 * @param speed Speed from 0 to 100
 * @return ESP_OK on success
 */
esp_err_t motor_drive_forward(uint8_t speed);

/**
 * @brief Move backward at specified speed
 * 
 * @param speed Speed from 0 to 100
 * @return ESP_OK on success
 */
esp_err_t motor_drive_backward(uint8_t speed);

/**
 * @brief Stop drive motor
 * 
 * @return ESP_OK on success
 */
esp_err_t motor_drive_stop(void);

/**
 * @brief Set steering position
 * 
 * @param position Preset steering position
 * @return ESP_OK on success
 */
esp_err_t motor_steering_set_position(steering_position_t position);

/**
 * @brief Set steering angle
 * 
 * @param angle Angle from -100 (full left) to +100 (full right), 0 = center
 * @return ESP_OK on success
 */
esp_err_t motor_steering_set_angle(int8_t angle);

/**
 * @brief Center steering
 * 
 * @return ESP_OK on success
 */
esp_err_t motor_steering_center(void);

/**
 * @brief Move forward with steering
 * 
 * @param speed Speed from 0 to 100
 * @param steering_angle Steering angle from -100 (left) to +100 (right)
 * @return ESP_OK on success
 */
esp_err_t motor_move_forward(uint8_t speed, int8_t steering_angle);

/**
 * @brief Move backward with steering
 * 
 * @param speed Speed from 0 to 100
 * @param steering_angle Steering angle from -100 (left) to +100 (right)
 * @return ESP_OK on success
 */
esp_err_t motor_move_backward(uint8_t speed, int8_t steering_angle);

/**
 * @brief Turn left while moving forward
 * 
 * @param speed Speed from 0 to 100
 * @return ESP_OK on success
 */
esp_err_t motor_turn_left(uint8_t speed);

/**
 * @brief Turn right while moving forward
 * 
 * @param speed Speed from 0 to 100
 * @return ESP_OK on success
 */
esp_err_t motor_turn_right(uint8_t speed);

/**
 * @brief Stop all motors
 * 
 * @return ESP_OK on success
 */
esp_err_t motor_stop_all(void);

/**
 * @brief Deinitialize motor control system
 * 
 * @return ESP_OK on success
 */
esp_err_t motor_control_deinit(void);

#ifdef __cplusplus
}
#endif

#endif // MOTOR_CONTROL_H
