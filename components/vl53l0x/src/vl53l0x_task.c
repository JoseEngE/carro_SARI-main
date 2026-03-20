/*
 * VL53L0X FreeRTOS Task Implementation
 * Provides continuous measurement with queue output
 */

#include "vl53l0x_task.h"
#include "esp_log.h"
#include <stdlib.h>
#include <string.h>

static const char *TAG = "VL53L0X_TASK";

/**
 * @brief Internal task context structure
 */
typedef struct {
    VL53L0X_Dev_t device;
    vl53l0x_task_config_t config;
    TaskHandle_t task_handle;
    bool running;
} vl53l0x_task_context_t;

/**
 * @brief Main VL53L0X measurement task
 */
static void vl53l0x_measurement_task(void *pvParameters) {
    vl53l0x_task_context_t *ctx = (vl53l0x_task_context_t *)pvParameters;
    VL53L0X_Error status;
    VL53L0X_DEV Dev = &ctx->device;
    
    ESP_LOGI(TAG, "VL53L0X task started on core %d", xPortGetCoreID());
    
    // Initialize I2C platform
    status = VL53L0X_PlatformInit(ctx->config.i2c_port, 
                                   ctx->config.sda_pin, 
                                   ctx->config.scl_pin, 
                                   ctx->config.i2c_speed_hz);
    if (status != VL53L0X_ERROR_NONE) {
        ESP_LOGE(TAG, "Platform init failed: %d", status);
        ctx->running = false;
        vTaskDelete(NULL);
        return;
    }
    
    // Wait for sensor power-up stabilization
    ESP_LOGI(TAG, "Waiting for sensor stabilization...");
    vTaskDelay(pdMS_TO_TICKS(50));
    
    // Configure device structure
    Dev->DeviceSpecificParameters.I2cDevAddr = ctx->config.device_address;
    Dev->DeviceSpecificParameters.CommsType = 1;  // I2C
    Dev->DeviceSpecificParameters.CommsSpeedKhz = ctx->config.i2c_speed_hz / 1000;
    
    // Data Initialization
    status = VL53L0X_DataInit(Dev);
    if (status != VL53L0X_ERROR_NONE) {
        ESP_LOGE(TAG, "Data init failed: %d", status);
        goto cleanup;
    }
    
    // Static Initialization
    status = VL53L0X_StaticInit(Dev);
    if (status != VL53L0X_ERROR_NONE) {
        ESP_LOGE(TAG, "Static init failed: %d", status);
        goto cleanup;
    }
    
    // Perform Reference Calibration (using main API implementation)
    uint8_t VhvSettings, PhaseCal;
    // ESP_LOGW(TAG, "╔════════════════════════════════════════════════╗");
    // ESP_LOGW(TAG, "║  Using DEBUG calibration - detailed logging    ║");
    // ESP_LOGW(TAG, "╚════════════════════════════════════════════════╝");
    status = VL53L0X_PerformRefCalibration(Dev, &VhvSettings, &PhaseCal);
    if (status != VL53L0X_ERROR_NONE) {
        ESP_LOGE(TAG, "Ref calibration failed: %d", status);
        goto cleanup;
    }
    
    // Set measurement mode
    switch (ctx->config.measurement_mode) {
        case VL53L0X_MODE_HIGH_ACCURACY:
            status = VL53L0X_SetHighAccuracyMode(Dev);
            break;
        case VL53L0X_MODE_LONG_RANGE:
            status = VL53L0X_SetLongRangeMode(Dev);
            break;
        case VL53L0X_MODE_HIGH_SPEED:
            status = VL53L0X_SetHighSpeedMode(Dev);
            break;
        case VL53L0X_MODE_DEFAULT:
        default:
            status = VL53L0X_SetDefaultMode(Dev);
            break;
    }
    
    if (status != VL53L0X_ERROR_NONE) {
        ESP_LOGE(TAG, "Mode config failed: %d", status);
        goto cleanup;
    }
    
    // Set continuous ranging mode
    status = VL53L0X_SetDeviceMode(Dev, VL53L0X_DEVICEMODE_CONTINUOUS_RANGING);
    if (status != VL53L0X_ERROR_NONE) {
        ESP_LOGE(TAG, "Set device mode failed: %d", status);
        goto cleanup;
    }
    
    // Set inter-measurement period
    status = VL53L0X_SetInterMeasurementPeriodMilliSeconds(Dev, ctx->config.measurement_interval_ms);
    if (status != VL53L0X_ERROR_NONE) {
        ESP_LOGE(TAG, "Set inter-measurement period failed: %d", status);
        goto cleanup;
    }
    
    // Start measurement
    status = VL53L0X_StartMeasurement(Dev);
    if (status != VL53L0X_ERROR_NONE) {
        ESP_LOGE(TAG, "Start measurement failed: %d", status);
        goto cleanup;
    }
    
    ESP_LOGI(TAG, "VL53L0X initialized successfully, starting measurements...");
    
    // Main measurement loop
    while (ctx->running) {
        VL53L0X_RangingMeasurementData_t measurement_data;
        uint32_t interrupt_status;
        
        // Wait for measurement data ready
        uint16_t timeout = 0;
        do {
            status = VL53L0X_GetInterruptMaskStatus(Dev, &interrupt_status);
            if (status != VL53L0X_ERROR_NONE) {
                ESP_LOGE(TAG, "Get interrupt status failed: %d", status);
                break;
            }
            
            if (timeout++ > 1000) {
                ESP_LOGW(TAG, "Measurement timeout");
                break;
            }
            vTaskDelay(pdMS_TO_TICKS(1));
        } while ((interrupt_status & 0x07) == 0 && ctx->running);
        
        if (!ctx->running) break;
        
        // Get measurement data
        status = VL53L0X_GetRangingMeasurementData(Dev, &measurement_data);
        if (status != VL53L0X_ERROR_NONE) {
            ESP_LOGE(TAG, "Get measurement data failed: %d", status);
            continue;
        }
        
        // Clear interrupt
        status = VL53L0X_ClearInterruptMask(Dev, 0);
        if (status != VL53L0X_ERROR_NONE) {
            ESP_LOGW(TAG, "Clear interrupt failed: %d", status);
        }
        
        // Prepare measurement data for queue
        vl53l0x_measurement_t queue_data = {
            .timestamp_ms = measurement_data.TimeStamp,
            .range_mm = measurement_data.RangeMilliMeter,
            .range_status = measurement_data.RangeStatus,
            .signal_rate_mcps = measurement_data.SignalRateRtnMegaCps,
            .ambient_rate_mcps = measurement_data.AmbientRateRtnMegaCps,
            .effective_spad_count = measurement_data.EffectiveSpadRtnCount
        };
        
        // Send to queue (non-blocking)
        if (ctx->config.measurement_queue != NULL) {
            if (xQueueSend(ctx->config.measurement_queue, &queue_data, 0) != pdTRUE) {
                ESP_LOGW(TAG, "Queue full, measurement dropped");
            }
        }
        
        // Log measurement (only valid ones to reduce spam)
        if (measurement_data.RangeStatus == VL53L0X_RANGESTATUS_RANGE_VALID) {
            ESP_LOGD(TAG, "Range: %d mm, Status: %d", 
                     measurement_data.RangeMilliMeter, 
                     measurement_data.RangeStatus);
        }
    }
    
    // Stop measurement
    VL53L0X_StopMeasurement(Dev);
    
cleanup:
    VL53L0X_PlatformDeInit(ctx->config.i2c_port);
    ctx->running = false;
    ESP_LOGI(TAG, "VL53L0X task stopped");
    vTaskDelete(NULL);
}

VL53L0X_Error vl53l0x_task_create(const vl53l0x_task_config_t *config, vl53l0x_task_handle_t *handle) {
    if (config == NULL || handle == NULL) {
        return VL53L0X_ERROR_INVALID_PARAMS;
    }
    
    // Allocate task context
    vl53l0x_task_context_t *ctx = (vl53l0x_task_context_t *)malloc(sizeof(vl53l0x_task_context_t));
    if (ctx == NULL) {
        ESP_LOGE(TAG, "Failed to allocate task context");
        return VL53L0X_ERROR_BUFFER_TOO_SMALL;
    }
    
    memset(ctx, 0, sizeof(vl53l0x_task_context_t));
    memcpy(&ctx->config, config, sizeof(vl53l0x_task_config_t));
    ctx->running = true;
    
    // Create FreeRTOS task
    BaseType_t result = xTaskCreate(
        vl53l0x_measurement_task,
        config->task_name,
        config->task_stack_size,
        ctx,
        config->task_priority,
        &ctx->task_handle
    );
    
    if (result != pdPASS) {
        ESP_LOGE(TAG, "Failed to create task");
        free(ctx);
        return VL53L0X_ERROR_UNDEFINED;
    }
    
    *handle = ctx;
    ESP_LOGI(TAG, "VL53L0X task created: %s", config->task_name);
    
    return VL53L0X_ERROR_NONE;
}

VL53L0X_Error vl53l0x_task_delete(vl53l0x_task_handle_t handle) {
    if (handle == NULL) {
        return VL53L0X_ERROR_INVALID_PARAMS;
    }
    
    vl53l0x_task_context_t *ctx = (vl53l0x_task_context_t *)handle;
    
    // Signal task to stop
    ctx->running = false;
    
    // Wait for task to finish (with timeout)
    uint16_t timeout = 0;
    while (eTaskGetState(ctx->task_handle) != eDeleted && timeout++ < 100) {
        vTaskDelay(pdMS_TO_TICKS(10));
    }
    
    // Free context
    free(ctx);
    
    ESP_LOGI(TAG, "VL53L0X task deleted");
    
    return VL53L0X_ERROR_NONE;
}

void vl53l0x_task_get_default_config(vl53l0x_task_config_t *config) {
    if (config == NULL) return;
    
    memset(config, 0, sizeof(vl53l0x_task_config_t));
    
    config->i2c_port = I2C_NUM_0;
    config->sda_pin = 21;
    config->scl_pin = 22;
    config->i2c_speed_hz = 400000;  // 400 kHz
    config->device_address = VL53L0X_DEFAULT_ADDRESS;
    
    config->measurement_mode = VL53L0X_MODE_DEFAULT;
    config->measurement_interval_ms = 100;  // 100ms
    
    config->measurement_queue = NULL;
    
    config->task_name = "vl53l0x_task";
    config->task_stack_size = 4096;
    config->task_priority = 5;
}
