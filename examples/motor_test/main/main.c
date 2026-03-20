/**
 * @file main.c
 * @brief Motor Test Example - Individual Motor Testing
 * 
 * Tests drive and steering motors independently, then combined movements
 */

#include <stdio.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "esp_log.h"
#include "motor_control.h"

static const char *TAG = "MOTOR_TEST";

void app_main(void) {
    ESP_LOGI(TAG, "");
    ESP_LOGI(TAG, "╔════════════════════════════════════════╗");
    ESP_LOGI(TAG, "║    RC Car Motor Test - MX1508          ║");
    ESP_LOGI(TAG, "╚════════════════════════════════════════╝");
    ESP_LOGI(TAG, "");
    
    // Configure drive motor (rear)
    motor_config_t drive_config = {
        .in1_pin = GPIO_NUM_7,
        .in2_pin = GPIO_NUM_8,
        .pwm_freq_hz = 1000,
        .timer = LEDC_TIMER_0,
        .channel_a = LEDC_CHANNEL_0,
        .channel_b = LEDC_CHANNEL_1
    };
    
    // Configure steering motor (front)
    motor_config_t steering_config = {
        .in1_pin = GPIO_NUM_9,
        .in2_pin = GPIO_NUM_10,
        .pwm_freq_hz = 1000,
        .timer = LEDC_TIMER_1,
        .channel_a = LEDC_CHANNEL_2,
        .channel_b = LEDC_CHANNEL_3
    };
    
    // Initialize motors
    ESP_LOGI(TAG, "Initializing motors...");
    esp_err_t ret = motor_control_init(&drive_config, &steering_config);
    if (ret != ESP_OK) {
        ESP_LOGE(TAG, "Failed to initialize motors!");
        return;
    }
    
    ESP_LOGI(TAG, "Motors initialized successfully");
    ESP_LOGI(TAG, "");
    vTaskDelay(pdMS_TO_TICKS(1000));
    
    // ========== TEST 1: Drive Motor ==========
    ESP_LOGI(TAG, "╔════════════════════════════════════════╗");
    ESP_LOGI(TAG, "║  TEST 1: Drive Motor (Rear)            ║");
    ESP_LOGI(TAG, "╚════════════════════════════════════════╝");
    ESP_LOGI(TAG, "");
    
    ESP_LOGI(TAG, "→ Forward 30%%");
    motor_drive_forward(30);
    vTaskDelay(pdMS_TO_TICKS(2000));
    motor_drive_stop();
    vTaskDelay(pdMS_TO_TICKS(500));
    
    ESP_LOGI(TAG, "→ Forward 50%%");
    motor_drive_forward(50);
    vTaskDelay(pdMS_TO_TICKS(2000));
    motor_drive_stop();
    vTaskDelay(pdMS_TO_TICKS(500));
    
    ESP_LOGI(TAG, "→ Forward 70%%");
    motor_drive_forward(70);
    vTaskDelay(pdMS_TO_TICKS(2000));
    motor_drive_stop();
    vTaskDelay(pdMS_TO_TICKS(1000));
    
    ESP_LOGI(TAG, "← Backward 30%%");
    motor_drive_backward(30);
    vTaskDelay(pdMS_TO_TICKS(2000));
    motor_drive_stop();
    vTaskDelay(pdMS_TO_TICKS(500));
    
    ESP_LOGI(TAG, "← Backward 50%%");
    motor_drive_backward(50);
    vTaskDelay(pdMS_TO_TICKS(2000));
    motor_drive_stop();
    vTaskDelay(pdMS_TO_TICKS(1000));
    
    ESP_LOGI(TAG, "✓ Drive motor test complete");
    ESP_LOGI(TAG, "");
    vTaskDelay(pdMS_TO_TICKS(2000));
    
    // ========== TEST 2: Steering Motor ==========
    ESP_LOGI(TAG, "╔════════════════════════════════════════╗");
    ESP_LOGI(TAG, "║  TEST 2: Steering Motor (Front)        ║");
    ESP_LOGI(TAG, "╚════════════════════════════════════════╝");
    ESP_LOGI(TAG, "");
    
    ESP_LOGI(TAG, "⊙ Center position");
    motor_steering_center();
    vTaskDelay(pdMS_TO_TICKS(1500));
    
    ESP_LOGI(TAG, "← Left 50%%");
    motor_steering_set_angle(-50);
    vTaskDelay(pdMS_TO_TICKS(1500));
    
    ESP_LOGI(TAG, "⊙ Center");
    motor_steering_center();
    vTaskDelay(pdMS_TO_TICKS(1500));
    
    ESP_LOGI(TAG, "→ Right 50%%");
    motor_steering_set_angle(50);
    vTaskDelay(pdMS_TO_TICKS(1500));
    
    ESP_LOGI(TAG, "⊙ Center");
    motor_steering_center();
    vTaskDelay(pdMS_TO_TICKS(1500));
    
    ESP_LOGI(TAG, "← Full left");
    motor_steering_set_position(STEER_LEFT);
    vTaskDelay(pdMS_TO_TICKS(1500));
    
    ESP_LOGI(TAG, "⊙ Center");
    motor_steering_center();
    vTaskDelay(pdMS_TO_TICKS(1500));
    
    ESP_LOGI(TAG, "→ Full right");
    motor_steering_set_position(STEER_RIGHT);
    vTaskDelay(pdMS_TO_TICKS(1500));
    
    ESP_LOGI(TAG, "⊙ Center");
    motor_steering_center();
    vTaskDelay(pdMS_TO_TICKS(1000));
    
    ESP_LOGI(TAG, "✓ Steering motor test complete");
    ESP_LOGI(TAG, "");
    vTaskDelay(pdMS_TO_TICKS(2000));
    
    // ========== TEST 3: Combined Movement ==========
    ESP_LOGI(TAG, "╔════════════════════════════════════════╗");
    ESP_LOGI(TAG, "║  TEST 3: Combined Movement             ║");
    ESP_LOGI(TAG, "╚════════════════════════════════════════╝");
    ESP_LOGI(TAG, "");
    
    ESP_LOGI(TAG, "↑ Forward straight (60%%)");
    motor_move_forward(60, 0);
    vTaskDelay(pdMS_TO_TICKS(2000));
    motor_stop_all();
    vTaskDelay(pdMS_TO_TICKS(1000));
    
    ESP_LOGI(TAG, "↖ Forward + Left turn");
    motor_turn_left(50);
    vTaskDelay(pdMS_TO_TICKS(2000));
    motor_stop_all();
    vTaskDelay(pdMS_TO_TICKS(1000));
    
    ESP_LOGI(TAG, "↗ Forward + Right turn");
    motor_turn_right(50);
    vTaskDelay(pdMS_TO_TICKS(2000));
    motor_stop_all();
    vTaskDelay(pdMS_TO_TICKS(1000));
    
    ESP_LOGI(TAG, "↙ Backward + Left");
    motor_move_backward(40, -60);
    vTaskDelay(pdMS_TO_TICKS(2000));
    motor_stop_all();
    vTaskDelay(pdMS_TO_TICKS(1000));
    
    ESP_LOGI(TAG, "↘ Backward + Right");
    motor_move_backward(40, 60);
    vTaskDelay(pdMS_TO_TICKS(2000));
    motor_stop_all();
    vTaskDelay(pdMS_TO_TICKS(1000));
    
    ESP_LOGI(TAG, "✓ Combined movement test complete");
    ESP_LOGI(TAG, "");
    
    // Final message
    ESP_LOGI(TAG, "");
    ESP_LOGI(TAG, "╔════════════════════════════════════════╗");
    ESP_LOGI(TAG, "║  All Tests Completed Successfully!     ║");
    ESP_LOGI(TAG, "╚════════════════════════════════════════╝");
    ESP_LOGI(TAG, "");
    ESP_LOGI(TAG, "Motors are now stopped.");
    ESP_LOGI(TAG, "You can now integrate with sensors for autonomous driving.");
}
