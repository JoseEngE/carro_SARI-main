/*
 * VL53L0X FreeRTOS Task Wrapper for ESP32
 * Provides easy integration with FreeRTOS applications
 */

#ifndef VL53L0X_TASK_H_
#define VL53L0X_TASK_H_

#include "vl53l0x.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/queue.h"

#ifdef __cplusplus
extern "C" {
#endif

/**
 * @brief Measurement mode options
 */
typedef enum {
    VL53L0X_MODE_DEFAULT = 0,      /*!< Default balanced mode */
    VL53L0X_MODE_HIGH_ACCURACY,    /*!< High accuracy mode */
    VL53L0X_MODE_LONG_RANGE,       /*!< Long range mode (~2m) */
    VL53L0X_MODE_HIGH_SPEED        /*!< High speed mode */
} vl53l0x_mode_t;

/**
 * @brief Measurement data structure for queue
 */
typedef struct {
    uint32_t timestamp_ms;          /*!< Measurement timestamp */
    uint16_t range_mm;              /*!< Distance in millimeters */
    uint8_t range_status;           /*!< Range status (0 = valid) */
    uint32_t signal_rate_mcps;      /*!< Signal rate in mega counts per second */
    uint32_t ambient_rate_mcps;     /*!< Ambient rate in mega counts per second */
    uint16_t effective_spad_count;  /*!< Effective SPAD return count */
} vl53l0x_measurement_t;

/**
 * @brief VL53L0X Task Configuration
 */
typedef struct {
    i2c_port_t i2c_port;            /*!< I2C port number */
    int sda_pin;                    /*!< I2C SDA pin */
    int scl_pin;                    /*!< I2C SCL pin */
    uint32_t i2c_speed_hz;          /*!< I2C speed in Hz */
    uint8_t device_address;         /*!< VL53L0X I2C address (default 0x29) */
    
    vl53l0x_mode_t measurement_mode; /*!< Measurement mode */
    uint32_t measurement_interval_ms; /*!< Measurement interval in milliseconds */
    
    QueueHandle_t measurement_queue; /*!< Queue to send measurements to */
    
    const char *task_name;          /*!< Task name */
    uint32_t task_stack_size;       /*!< Task stack size in bytes */
    UBaseType_t task_priority;      /*!< Task priority */
} vl53l0x_task_config_t;

/**
 * @brief VL53L0X Task Handle
 */
typedef void* vl53l0x_task_handle_t;

/**
 * @brief Create and start VL53L0X measurement task
 * 
 * @param config Pointer to task configuration
 * @param handle Pointer to store task handle
 * @return VL53L0X_ERROR_NONE on success
 */
VL53L0X_Error vl53l0x_task_create(const vl53l0x_task_config_t *config, vl53l0x_task_handle_t *handle);

/**
 * @brief Stop and delete VL53L0X measurement task
 * 
 * @param handle Task handle
 * @return VL53L0X_ERROR_NONE on success
 */
VL53L0X_Error vl53l0x_task_delete(vl53l0x_task_handle_t handle);

/**
 * @brief Get default task configuration
 * 
 * @param config Pointer to configuration structure to fill
 */
void vl53l0x_task_get_default_config(vl53l0x_task_config_t *config);

#ifdef __cplusplus
}
#endif

#endif /* VL53L0X_TASK_H_ */
