#include <stdio.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "esp_log.h"
#include "sensor_manager.h"
#include "ble_serial.h"

#define TAG "MAIN_APP"

// I2C Configuration
#define I2C_SDA_PIN 10
#define I2C_SCL_PIN 9
#define I2C_FREQ_HZ 400000

// Sensor Configuration
#define SENSOR_COUNT 3

VL53L0X_Dev_t dev1, dev2, dev3;

SensorConfig_t sensors[SENSOR_COUNT] = {
    { .xshut_pin = 14, .i2c_address = 0x30, .device = &dev1, .offset_mm = OFFSET_SENSOR_RIGHT, .active = false }, // S1
    { .xshut_pin = 13, .i2c_address = 0x31, .device = &dev2, .offset_mm = OFFSET_SENSOR_CENTER, .active = false }, // S2
    { .xshut_pin = 12, .i2c_address = 0x32, .device = &dev3, .offset_mm = OFFSET_SENSOR_LEFT, .active = false }  // S3
};

// process_decision removed per user request

void measurement_task(void *pvParameters) {
    ESP_LOGI(TAG, "Starting measurement task...");

    // Start Continuous Mode on all active sensors
    sensor_manager_start_continuous(sensors, SENSOR_COUNT);
    
    VL53L0X_RangingMeasurementData_t results[SENSOR_COUNT];

    while (1) {
        // 1. Read data from all sensors
        sensor_manager_read_all(sensors, SENSOR_COUNT, results);

        // 3. Print Results
        printf("S1: %4d mm (St:%d) | S2: %4d mm (St:%d) | S3: %4d mm (St:%d)\n",
            sensors[0].active ? results[0].RangeMilliMeter : -1, results[0].RangeStatus,
            sensors[1].active ? results[1].RangeMilliMeter : -1, results[1].RangeStatus,
            sensors[2].active ? results[2].RangeMilliMeter : -1, results[2].RangeStatus
        );

        // Send to BLE if connected
        ble_serial_print("S1: %4d mm | S2: %4d mm | S3: %4d mm\n",
            sensors[0].active ? results[0].RangeMilliMeter : -1,
            sensors[1].active ? results[1].RangeMilliMeter : -1,
            sensors[2].active ? results[2].RangeMilliMeter : -1
        );

        vTaskDelay(pdMS_TO_TICKS(100));
    }
}

void app_main(void) {
    // Determine the log level
    esp_log_level_set("VL53L0X_API", ESP_LOG_WARN);
    esp_log_level_set("VL53L0X_PLATFORM", ESP_LOG_WARN);
    esp_log_level_set("SENSOR_MANAGER", ESP_LOG_INFO);
    
    // Initialize BLE Serial Logger
    ble_serial_init();

    ESP_LOGI(TAG, "Initializing Sensor Manager...");

    // Initialize Manager (Bus + Sensors)
    if (sensor_manager_init(sensors, SENSOR_COUNT, I2C_SDA_PIN, I2C_SCL_PIN, I2C_FREQ_HZ, SENSOR_MODE_HIGH_PRECISION) == ESP_OK) {
        xTaskCreate(measurement_task, "measure_task", 4096, NULL, 5, NULL);
    } else {
        ESP_LOGE(TAG, "Critical Failure in Sensor Manager Init");
    }
}
