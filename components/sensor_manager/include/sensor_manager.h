#pragma once

#include "driver/gpio.h"
#include "driver/i2c_master.h"
#include "vl53l0x.h"

#ifdef __cplusplus
extern "C" {
#endif

typedef enum {
    SENSOR_MODE_DEFAULT,
    SENSOR_MODE_HIGH_PRECISION // Optimized for short range, high accuracy
} SensorMode_t;

typedef struct {
    gpio_num_t xshut_pin;
    uint8_t i2c_address;
    VL53L0X_Dev_t *device; // Pointer to the device struct to be initialized
    int16_t offset_mm;
    bool active;           // Output: set to true if sensor is successfully initialized
} SensorConfig_t;

// --- User Calibration Offsets ---
#define OFFSET_SENSOR_1 1
#define OFFSET_SENSOR_2 1
#define OFFSET_SENSOR_3 1

/**
 * @brief Initialize the I2C Master Bus and all sensors
 * 
 * @param configs Array of sensor configurations
 * @param count Number of sensors
 * @param sda_pin I2C SDA Pin
 * @param scl_pin I2C SCL Pin
 * @param speed_hz I2C Frequency
 * @param mode Sensor operation mode
 * @return esp_err_t ESP_OK on success
 */
esp_err_t sensor_manager_init(SensorConfig_t *configs, int count, int sda_pin, int scl_pin, uint32_t speed_hz, SensorMode_t mode);

/**
 * @brief Start continuous measurements on all active sensors
 * 
 * @param configs Array of sensor configurations
 * @param count Number of sensors
 */
void sensor_manager_start_continuous(SensorConfig_t *configs, int count);

/**
 * @brief Read data from all active sensors
 * 
 * @param configs Array of sensor configurations
 * @param count Number of sensors
 * @param data Array of measurement data structs to fill
 */
void sensor_manager_read_all(SensorConfig_t *configs, int count, VL53L0X_RangingMeasurementData_t *data);

#ifdef __cplusplus
}
#endif
