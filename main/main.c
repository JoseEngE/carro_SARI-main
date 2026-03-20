/**
 * @file main.c
 * @brief RC Car Web Control - Main Application
 * 
 * Web-based control for RC car with mobile interface
 */

#include <stdio.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "esp_log.h"
#include "motor_control.h"
#include "web_control.h"
#include "servo_control.h"

#include "wall_follower.h"
#include "sensor_manager.h"
#include "ble_serial.h"

static const char *TAG = "MAIN";

// I2C Configuration
#define I2C_SDA_PIN 10
#define I2C_SCL_PIN 9
#define I2C_FREQ_HZ 400000

// Sensor Configuration
#define SENSOR_COUNT 3

VL53L0X_Dev_t dev1, dev2, dev3;

SensorConfig_t sensors[SENSOR_COUNT] = {
    { .xshut_pin = 14, .i2c_address = 0x30, .device = &dev1, .offset_mm = OFFSET_SENSOR_1, .active = false }, // S1 (Right)
    { .xshut_pin = 13, .i2c_address = 0x31, .device = &dev2, .offset_mm = OFFSET_SENSOR_2, .active = false }, // S2 (Center)
    { .xshut_pin = 12, .i2c_address = 0x32, .device = &dev3, .offset_mm = OFFSET_SENSOR_3, .active = false }  // S3 (Left)
};

// System State
volatile bool autonomous_mode = false;
volatile uint8_t global_speed_limit = 50;

float map_range(float x, float in_min, float in_max, float out_min, float out_max) {
    return (x - in_min) * (out_max - out_min) / (in_max - in_min) + out_min;
}

void auto_navigation_task(void *pvParameters) {
    ESP_LOGI(TAG, "Starting Auto Navigation task...");
    wall_follower_config_t wf_config = wall_follower_get_default_config();
    wall_follower_init(&wf_config);
    
    sensor_manager_start_continuous(sensors, SENSOR_COUNT);
    VL53L0X_RangingMeasurementData_t results[SENSOR_COUNT];

    while (1) {
        if (autonomous_mode) {
            sensor_manager_read_all(sensors, SENSOR_COUNT, results);

            uint16_t d_right = sensors[0].active ? results[0].RangeMilliMeter : 8190;
            uint16_t d_center = sensors[1].active ? results[1].RangeMilliMeter : 8190;
            uint16_t d_left = sensors[2].active ? results[2].RangeMilliMeter : 8190;

            int8_t auto_speed = 0;
            int8_t auto_steering = 0;
            wall_follower_process(d_left, d_center, d_right, &auto_speed, &auto_steering);

            // Apply the actual movement
            if (auto_speed > 0) {
                // Apply global speed limit percentage
                uint8_t final_speed = (auto_speed * global_speed_limit) / 100;
                motor_drive_forward(final_speed);
            } else {
                motor_drive_stop();
            }
            // Map the steering
            float mapped_angle = map_range((float)auto_steering, -100.0f, 100.0f, 75.0f, 41.0f);
            servo_set_angle(mapped_angle);

            // Send to BLE
            ble_serial_print("AutoNAV | L:%4d C:%4d R:%4d | Spd:%d Str:%d\n", d_left, d_center, d_right, auto_speed, auto_steering);
            
            // Print to Serial Monitor
            ESP_LOGI(TAG, "L:%4d C:%4d R:%4d | Spd:%d Str:%d", d_left, d_center, d_right, auto_speed, auto_steering);
        }

        vTaskDelay(pdMS_TO_TICKS(50));
    }
}

/**
 * @brief Motor control callback - called when web commands are received
 */

void mode_callback(uint8_t mode) {
    if (mode == 1) {
        autonomous_mode = true;
        ESP_LOGI(TAG, "Switched to AUTONOMOUS Mode");
    } else {
        autonomous_mode = false;
        motor_drive_stop();
        servo_set_angle(58.0f); // Center
        ESP_LOGI(TAG, "Switched to MANUAL Mode");
    }
}

void speed_limit_callback(uint8_t limit) {
    if (limit > 100) limit = 100;
    global_speed_limit = limit;
    ESP_LOGI(TAG, "Global speed limit updated to: %d%%", limit);
}

void motor_callback(int8_t throttle, int8_t steering)
{
    if (autonomous_mode) return; // Ignore manual commands in auto mode

    ESP_LOGI(TAG, "Motor command: throttle=%d, steering=%d", throttle, steering);
    
    // Apply throttle (forward/backward)
    if (throttle > 5) {
        motor_drive_forward(throttle);
    } else if (throttle < -5) {
        motor_drive_backward(-throttle);
    } else {
        motor_drive_stop();
    }
    
    // Apply steering (servo)
    // Map -100 to 100 range to 75-41 degrees
    float angle = map_range((float)steering, -100.0f, 100.0f, 75.0f, 41.0f);
    servo_set_angle(angle);
}

void app_main(void) {
    ESP_LOGI(TAG, "");
    ESP_LOGI(TAG, "╔════════════════════════════════════════╗");
    ESP_LOGI(TAG, "║    RC Car Web Control System           ║");
    ESP_LOGI(TAG, "╚════════════════════════════════════════╝");
    ESP_LOGI(TAG, "");
    
    // Configure drive motor (rear)
    motor_config_t drive_config = {
        .in1_pin = GPIO_NUM_2,
        .in2_pin = GPIO_NUM_42,
        .pwm_freq_hz = 1000,
        .timer = LEDC_TIMER_0,
        .channel_a = LEDC_CHANNEL_0,
        .channel_b = LEDC_CHANNEL_1
    };
    
    // Initialize drive motor
    ESP_LOGI(TAG, "Initializing drive motor...");
    esp_err_t ret = motor_control_init(&drive_config, NULL); // Pass NULL for steering config
    if (ret != ESP_OK) {
        ESP_LOGE(TAG, "Failed to initialize drive motor!");
        return;
    }
    ESP_LOGI(TAG, "✓ Drive motor initialized");

    // Initialize Servo
    // Using GPIO 9 (original steering pin) and parameters from servomotor example
    ESP_LOGI(TAG, "Initializing servo...");
    servo_config_t servo_cfg = {
        .gpio_num = GPIO_NUM_1,
        .timer_number = LEDC_TIMER_1, // Different timer than drive motor (Timer 0)
        .channel_number = LEDC_CHANNEL_2,
        .min_pulse_width_us = 500,
        .max_pulse_width_us = 2400,
        .frequency = 50,
        .min_angle = 41.0f,
        .max_angle = 75.0f,
        .initial_angle = 58.0f
    };
    ret = servo_init(&servo_cfg);
    if (ret != ESP_OK) {
        ESP_LOGE(TAG, "Failed to initialize servo!");
        return;
    }
    ESP_LOGI(TAG, "✓ Servo initialized");
    
    // Initialize web control
    ESP_LOGI(TAG, "Initializing web control...");
    web_control_config_t web_config = WEB_CONTROL_DEFAULT_CONFIG();
    ret = web_control_init(&web_config);
    if (ret != ESP_OK) {
        ESP_LOGE(TAG, "Failed to initialize web control!");
        return;
    }
    ESP_LOGI(TAG, "✓ Web control initialized");
    
    // Set callbacks
    web_control_set_motor_callback(motor_callback);
    web_control_set_mode_callback(mode_callback);
    web_control_set_speed_limit_callback(speed_limit_callback);
    
    // Start web server
    ESP_LOGI(TAG, "Starting web server...");
    ret = web_control_start();
    if (ret != ESP_OK) {
        ESP_LOGE(TAG, "Failed to start web server!");
        return;
    }
    
    ESP_LOGI(TAG, "");
    ESP_LOGI(TAG, "╔════════════════════════════════════════╗");
    ESP_LOGI(TAG, "║  Web Control Ready!                    ║");
    ESP_LOGI(TAG, "╚════════════════════════════════════════╝");
    ESP_LOGI(TAG, "");
    ESP_LOGI(TAG, "📱 Connect your phone to WiFi:");
    ESP_LOGI(TAG, "   SSID: RC_Car_Control");
    ESP_LOGI(TAG, "   Password: rccar123");
    ESP_LOGI(TAG, "");
    ESP_LOGI(TAG, "🌐 Open browser and go to:");
    ESP_LOGI(TAG, "   http://192.168.4.1");
    ESP_LOGI(TAG, "");
    
    // Initialize BLE Serial Logger
    ble_serial_init();

    ESP_LOGI(TAG, "Initializing Sensor Manager...");
    if (sensor_manager_init(sensors, SENSOR_COUNT, I2C_SDA_PIN, I2C_SCL_PIN, I2C_FREQ_HZ, SENSOR_MODE_HIGH_PRECISION) == ESP_OK) {
        xTaskCreate(auto_navigation_task, "auto_nav_task", 4096, NULL, 5, NULL);
    } else {
        ESP_LOGE(TAG, "Failed to initialize sensors for auto-navigation");
    }

    // Telemetry task - send data to web interface
    while (1) {
        if (web_control_is_connected()) {
            // Send telemetry (battery, speed, signal)
            // For now, using dummy values
            web_control_send_telemetry(87, 0.0, 100);
        }
        vTaskDelay(pdMS_TO_TICKS(100)); // Update every 100ms
    }
}
