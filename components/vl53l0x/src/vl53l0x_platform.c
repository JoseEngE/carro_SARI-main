/*
 * VL53L0X Platform Implementation for ESP32
 * I2C communication using the new driver/i2c_master.h
 */

#include "vl53l0x_platform.h"
#include "driver/i2c_master.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "esp_log.h"
#include <string.h>

static const char *TAG = "VL53L0X_PLATFORM";

// PlatformInit now just logs, as Bus creation is handled by the Sensor Manager
VL53L0X_Error VL53L0X_PlatformInit(i2c_port_t i2c_port, int sda_pin, int scl_pin, uint32_t speed_hz) {
    ESP_LOGI(TAG, "PlatformInit: I2C Bus should be initialized by Sensor Manager.");
    return VL53L0X_ERROR_NONE;
}

VL53L0X_Error VL53L0X_PlatformDeInit(i2c_port_t i2c_port) {
    return VL53L0X_ERROR_NONE;
}

VL53L0X_Error VL53L0X_WriteMulti(VL53L0X_DEV Dev, uint8_t index, uint8_t *pdata, uint32_t count) {
    if (Dev == NULL || pdata == NULL) {
        return VL53L0X_ERROR_INVALID_PARAMS;
    }

    i2c_master_dev_handle_t handle = (i2c_master_dev_handle_t)Dev->DeviceSpecificParameters.I2cHandle;
    if (handle == NULL) {
        ESP_LOGE(TAG, "I2C handle is NULL");
        return VL53L0X_ERROR_CONTROL_INTERFACE;
    }

    // Use stack buffer for small writes to avoid malloc overhead
    uint8_t stack_buffer[64];
    uint8_t *buffer = stack_buffer;
    bool use_malloc = false;

    if (count + 1 > sizeof(stack_buffer)) {
        buffer = (uint8_t *)malloc(count + 1);
        if (buffer == NULL) {
            return VL53L0X_ERROR_CONTROL_INTERFACE;
        }
        use_malloc = true;
    }

    buffer[0] = index;
    memcpy(buffer + 1, pdata, count);

    esp_err_t err = i2c_master_transmit(handle, buffer, count + 1, -1);

    if (use_malloc) {
        free(buffer);
    }

    if (err != ESP_OK) {
        ESP_LOGE(TAG, "I2C write failed at 0x%02X: %d", index, err);
        return VL53L0X_ERROR_CONTROL_INTERFACE;
    }

    return VL53L0X_ERROR_NONE;
}

VL53L0X_Error VL53L0X_ReadMulti(VL53L0X_DEV Dev, uint8_t index, uint8_t *pdata, uint32_t count) {
    if (Dev == NULL || pdata == NULL) {
        return VL53L0X_ERROR_INVALID_PARAMS;
    }

    i2c_master_dev_handle_t handle = (i2c_master_dev_handle_t)Dev->DeviceSpecificParameters.I2cHandle;
    if (handle == NULL) {
        return VL53L0X_ERROR_CONTROL_INTERFACE;
    }

    esp_err_t err = i2c_master_transmit_receive(handle, &index, 1, pdata, count, -1);

    if (err != ESP_OK) {
        ESP_LOGE(TAG, "I2C read failed at 0x%02X: %d", index, err);
        return VL53L0X_ERROR_CONTROL_INTERFACE;
    }

    return VL53L0X_ERROR_NONE;
}

VL53L0X_Error VL53L0X_WriteByte(VL53L0X_DEV Dev, uint8_t index, uint8_t data) {
    return VL53L0X_WriteMulti(Dev, index, &data, 1);
}

VL53L0X_Error VL53L0X_WriteWord(VL53L0X_DEV Dev, uint8_t index, uint16_t data) {
    uint8_t buffer[2];
    buffer[0] = (data >> 8) & 0xFF;  // MSB first
    buffer[1] = data & 0xFF;
    return VL53L0X_WriteMulti(Dev, index, buffer, 2);
}

VL53L0X_Error VL53L0X_WriteDWord(VL53L0X_DEV Dev, uint8_t index, uint32_t data) {
    uint8_t buffer[4];
    buffer[0] = (data >> 24) & 0xFF;  // MSB first
    buffer[1] = (data >> 16) & 0xFF;
    buffer[2] = (data >> 8) & 0xFF;
    buffer[3] = data & 0xFF;
    return VL53L0X_WriteMulti(Dev, index, buffer, 4);
}

VL53L0X_Error VL53L0X_ReadByte(VL53L0X_DEV Dev, uint8_t index, uint8_t *pdata) {
    return VL53L0X_ReadMulti(Dev, index, pdata, 1);
}

VL53L0X_Error VL53L0X_ReadWord(VL53L0X_DEV Dev, uint8_t index, uint16_t *pdata) {
    uint8_t buffer[2];
    VL53L0X_Error status = VL53L0X_ReadMulti(Dev, index, buffer, 2);
    if (status == VL53L0X_ERROR_NONE) {
        *pdata = ((uint16_t)buffer[0] << 8) | buffer[1];  // MSB first
    }
    return status;
}

VL53L0X_Error VL53L0X_ReadDWord(VL53L0X_DEV Dev, uint8_t index, uint32_t *pdata) {
    uint8_t buffer[4];
    VL53L0X_Error status = VL53L0X_ReadMulti(Dev, index, buffer, 4);
    if (status == VL53L0X_ERROR_NONE) {
        *pdata = ((uint32_t)buffer[0] << 24) | ((uint32_t)buffer[1] << 16) | 
                 ((uint32_t)buffer[2] << 8) | buffer[3];  // MSB first
    }
    return status;
}

VL53L0X_Error VL53L0X_PollingDelay(VL53L0X_DEV Dev, uint32_t ms) {
    (void)Dev;
    vTaskDelay(pdMS_TO_TICKS(ms));
    return VL53L0X_ERROR_NONE;
}

VL53L0X_Error VL53L0X_GetTickCount(uint32_t *ptick_count_ms) {
    if (ptick_count_ms == NULL) {
        return VL53L0X_ERROR_INVALID_PARAMS;
    }
    *ptick_count_ms = xTaskGetTickCount() * portTICK_PERIOD_MS;
    return VL53L0X_ERROR_NONE;
}
