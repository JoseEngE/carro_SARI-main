#pragma once

#include <stdint.h>
#include <stdbool.h>
#include "esp_err.h"

#ifdef __cplusplus
extern "C" {
#endif

/**
 * @brief Configuration parameters for the wall follower algorithm
 */
typedef struct {
    uint16_t min_front_dist_mm;     /*!< Distance to trigger sharp turns (e.g. 200 mm) */
    uint16_t side_wall_max_dist_mm; /*!< Maximum distance to rely on side sensors (e.g. 400 mm) */
    uint8_t base_speed;             /*!< Speed when moving forward (0-100) */
    uint8_t turn_speed;             /*!< Speed when making a sharp turn (0-100) */
    float kp_steering;              /*!< Proportional gain for steering correction (e.g. 0.3) */
} wall_follower_config_t;

/**
 * @brief Get the default configuration for the wall follower
 * 
 * @return wall_follower_config_t Default configuration
 */
wall_follower_config_t wall_follower_get_default_config(void);

/**
 * @brief Initialize the wall follower system
 * 
 * @param config Pointer to configuration struct
 * @return ESP_OK on success
 */
esp_err_t wall_follower_init(const wall_follower_config_t* config);

/**
 * @brief Process sensor distances and control motors to follow walls
 * 
 * This function should be called repeatedly in the measurement loop.
 * 
 * @param d_left Distance measured by left sensor (mm)
 * @param d_front Distance measured by front/center sensor (mm)
 * @param d_right Distance measured by right sensor (mm)
 * @param speed_out Pointer to output speed value
 * @param steering_out Pointer to output steering angle value
 */
void wall_follower_process(uint16_t d_left, uint16_t d_front, uint16_t d_right, int8_t *speed_out, int8_t *steering_out);

/**
 * @brief Stop following and stop motors
 */
void wall_follower_stop(void);

#ifdef __cplusplus
}
#endif
