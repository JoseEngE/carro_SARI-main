#include "wall_follower.h"
#include "motor_control.h"
#include "esp_log.h"
#include <math.h>

static const char *TAG = "WALL_FOLLOWER";

static wall_follower_config_t current_config;
static bool initialized = false;

wall_follower_config_t wall_follower_get_default_config(void) {
    wall_follower_config_t cfg = {
        .min_front_dist_mm = 350,      // Aumentado a 35 cm para que empiece a evadir mucho antes
        .side_wall_max_dist_mm = 500,  // Aumentado a 50 cm para que tenga un rango de visión lateral más amplio
        .base_speed = 70,              // 70% speed
        .turn_speed = 60,              // 60% speed for sharp turns
        .kp_steering = 1.0f            // Aumentado de 0.4 a 1.0 para que el servo doble más rápido y agresivo
    };
    return cfg;
}

esp_err_t wall_follower_init(const wall_follower_config_t* config) {
    if (config == NULL) {
        ESP_LOGE(TAG, "Config is NULL");
        return ESP_ERR_INVALID_ARG;
    }
    
    current_config = *config;
    initialized = true;
    ESP_LOGI(TAG, "Wall follower initialized. min_front=%d mm, base_speed=%d", 
             current_config.min_front_dist_mm, current_config.base_speed);
    
    return ESP_OK;
}

void wall_follower_process(uint16_t d_left, uint16_t d_front, uint16_t d_right, int8_t *speed_out, int8_t *steering_out) {
    if (!initialized) {
        ESP_LOGE(TAG, "Not initialized! Call wall_follower_init() first.");
        return;
    }

    // Handle invalid sensor readings (frequently > 8000 when out of range)
    if (d_left > 8000) d_left = current_config.side_wall_max_dist_mm;
    if (d_right > 8000) d_right = current_config.side_wall_max_dist_mm;
    if (d_front > 8000) d_front = 2000; // Far ahead

    // Cap side distances to max allowed threshold so errors don't spike uncontrollably
    if (d_left > current_config.side_wall_max_dist_mm) d_left = current_config.side_wall_max_dist_mm;
    if (d_right > current_config.side_wall_max_dist_mm) d_right = current_config.side_wall_max_dist_mm;

    // Check front distance first (Priority #1: Avoid Front Collision)
    if (d_front < current_config.min_front_dist_mm) {
        // Obstacle ahead, make a sharp turn towards the side with more space
        int8_t angle = 0;
        if (d_left > d_right) {
            // Left is more open, steer hard left
            angle = -100;
            ESP_LOGD(TAG, "Obstacle ahead (%d)! Turning LEFT", d_front);
        } else {
            // Right is more open or equal, steer hard right
            angle = 100;
            ESP_LOGD(TAG, "Obstacle ahead (%d)! Turning RIGHT", d_front);
        }
        
        // Output speed and steering
        if (speed_out) *speed_out = current_config.turn_speed;
        if (steering_out) *steering_out = angle;
    } else {
        // Path ahead is clear. Follow walls by comparing left and right distances.
        // Positive error means Left > Right (closer to Right wall) -> Need to steer Left (Negative angle)
        // Negative error means Left < Right (closer to Left wall) -> Need to steer Right (Positive angle)
        
        float diff = (float)d_left - (float)d_right;
        
        // Calculate angle. Since diff > 0 means closer to right wall, we need negative angle to turn left.
        // angle = -(diff * Kp)
        float val = -(diff * current_config.kp_steering);
        
        // Clamp to [-100, 100]
        if (val > 100.0f) val = 100.0f;
        if (val < -100.0f) val = -100.0f;
        
        int8_t angle = (int8_t)val;
        
        ESP_LOGD(TAG, "L:%d R:%d Diff:%.0f -> Angle:%d", d_left, d_right, diff, angle);
        
        if (speed_out) *speed_out = current_config.base_speed;
        if (steering_out) *steering_out = angle;
    }
}

void wall_follower_stop(void) {
    if (!initialized) return;
    ESP_LOGI(TAG, "Stopped wall follower");
}
