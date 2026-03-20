/*
 * VL53L0X Core API Implementation for ESP32
 * Adapted from STMicroelectronics VL53L0X API
 */

#include "vl53l0x.h"
#include "driver/gpio.h"
#include "esp_log.h"
#include <string.h>

static const char *TAG = "VL53L0X_API";

// VL53L0X Register Addresses
#define VL53L0X_REG_IDENTIFICATION_MODEL_ID         0xC0
#define VL53L0X_REG_IDENTIFICATION_REVISION_ID      0xC2
#define VL53L0X_REG_SYSRANGE_START                  0x00
#define VL53L0X_REG_SYSTEM_SEQUENCE_CONFIG          0x01
#define VL53L0X_REG_SYSTEM_RANGE_CONFIG             0x09
#define VL53L0X_REG_SYSTEM_INTERMEASUREMENT_PERIOD  0x04
#define VL53L0X_REG_SYSTEM_INTERRUPT_CONFIG_GPIO    0x0A
#define VL53L0X_REG_SYSTEM_INTERRUPT_CLEAR          0x0B
#define VL53L0X_REG_RESULT_INTERRUPT_STATUS         0x13
#define VL53L0X_REG_RESULT_RANGE_STATUS             0x14
#define VL53L0X_REG_RESULT_CORE_PAGE                0x24
#define VL53L0X_REG_I2C_SLAVE_DEVICE_ADDRESS        0x8A
#define VL53L0X_REG_MSRC_CONFIG_CONTROL             0x60
#define VL53L0X_REG_FINAL_RANGE_CONFIG_MIN_COUNT_RATE_RTN_LIMIT  0x44
#define VL53L0X_REG_CROSSTALK_COMPENSATION_PEAK_RATE_MCPS        0x20
#define VL53L0X_REG_ALGO_PHASECAL_LIM               0x30
#define VL53L0X_REG_ALGO_PHASECAL_CONFIG_TIMEOUT    0x30
#define VL53L0X_REG_GLOBAL_CONFIG_VCSEL_WIDTH       0x32
#define VL53L0X_REG_GLOBAL_CONFIG_SPAD_ENABLES_REF_0  0xB0
#define VL53L0X_REG_DYNAMIC_SPAD_REF_EN_START_OFFSET  0x4F
#define VL53L0X_REG_DYNAMIC_SPAD_NUM_REQUESTED_REF_SPAD 0x4E  
#define VL53L0X_REG_POWER_MANAGEMENT_GO1_POWER_FORCE  0x80
#define VL53L0X_REG_VHV_CONFIG_PAD_SCL_SDA__EXTSUP_HV 0x89
#define VL53L0X_REG_PRE_RANGE_CONFIG_VCSEL_PERIOD   0x50
#define VL53L0X_REG_PRE_RANGE_CONFIG_TIMEOUT_MACROP_HI  0x51
#define VL53L0X_REG_FINAL_RANGE_CONFIG_VCSEL_PERIOD 0x70
#define VL53L0X_REG_FINAL_RANGE_CONFIG_TIMEOUT_MACROP_HI  0x71
#define VL53L0X_REG_MSRC_CONFIG_TIMEOUT_MACROP      0x46

// Expected model ID
#define VL53L0X_EXPECTED_DEVICE_ID                  0xEE

VL53L0X_Error VL53L0X_DataInit(VL53L0X_DEV Dev) {
    VL53L0X_Error status = VL53L0X_ERROR_NONE;
    uint8_t model_id = 0;
    
    ESP_LOGI(TAG, "Initializing VL53L0X device...");
    
    // Read model ID to verify device
    status = VL53L0X_ReadByte(Dev, VL53L0X_REG_IDENTIFICATION_MODEL_ID, &model_id);
    if (status != VL53L0X_ERROR_NONE) {
        ESP_LOGE(TAG, "Failed to read model ID");
        return status;
    }
    
    if (model_id != VL53L0X_EXPECTED_DEVICE_ID) {
        ESP_LOGE(TAG, "Invalid model ID: 0x%02X (expected 0x%02X)", model_id, VL53L0X_EXPECTED_DEVICE_ID);
        return VL53L0X_ERROR_CONTROL_INTERFACE;
    }
    
    ESP_LOGI(TAG, "VL53L0X model ID verified: 0x%02X", model_id);
    
    // Set I2C standard mode
    status = VL53L0X_WriteByte(Dev, 0x88, 0x00);
    if (status != VL53L0X_ERROR_NONE) return status;
    
    status = VL53L0X_WriteByte(Dev, 0x80, 0x01);
    if (status != VL53L0X_ERROR_NONE) return status;
    
    status = VL53L0X_WriteByte(Dev, 0xFF, 0x01);
    if (status != VL53L0X_ERROR_NONE) return status;
    
    status = VL53L0X_WriteByte(Dev, 0x00, 0x00);
    if (status != VL53L0X_ERROR_NONE) return status;
    
    status = VL53L0X_WriteByte(Dev, 0xFF, 0x00);
    if (status != VL53L0X_ERROR_NONE) return status;
    
    status = VL53L0X_WriteByte(Dev, 0x80, 0x00);
    if (status != VL53L0X_ERROR_NONE) return status;
    
    // Initialize device parameters
    Dev->CurrentParameters.DeviceMode = VL53L0X_DEVICEMODE_SINGLE_RANGING;
    Dev->CurrentParameters.MeasurementTimingBudgetMicroSeconds = 33000;
    Dev->CurrentParameters.InterMeasurementPeriodMilliSeconds = 100;
    Dev->PalState = 1;  // Initialized
    
    ESP_LOGI(TAG, "VL53L0X data initialization complete");
    
    return VL53L0X_ERROR_NONE;
}

VL53L0X_Error VL53L0X_StaticInit(VL53L0X_DEV Dev) {
    VL53L0X_Error status = VL53L0X_ERROR_NONE;
    
    ESP_LOGI(TAG, "Static initialization...");
    
    // VL53L0X_load_tuning_settings
    status = VL53L0X_WriteByte(Dev, 0xFF, 0x01);
    if (status != VL53L0X_ERROR_NONE) return status;
    status = VL53L0X_WriteByte(Dev, 0x00, 0x00);
    if (status != VL53L0X_ERROR_NONE) return status;
    
    status = VL53L0X_WriteByte(Dev, 0xFF, 0x00);
    if (status != VL53L0X_ERROR_NONE) return status;
    status = VL53L0X_WriteByte(Dev, 0x09, 0x00);
    if (status != VL53L0X_ERROR_NONE) return status;
    status = VL53L0X_WriteByte(Dev, 0x10, 0x00);
    if (status != VL53L0X_ERROR_NONE) return status;
    status = VL53L0X_WriteByte(Dev, 0x11, 0x00);
    if (status != VL53L0X_ERROR_NONE) return status;
    
    status = VL53L0X_WriteByte(Dev, 0x24, 0x01);
    if (status != VL53L0X_ERROR_NONE) return status;
    status = VL53L0X_WriteByte(Dev, 0x25, 0xFF);
    if (status != VL53L0X_ERROR_NONE) return status;
    status = VL53L0X_WriteByte(Dev, 0x75, 0x00);
    if (status != VL53L0X_ERROR_NONE) return status;
    
    status = VL53L0X_WriteByte(Dev, 0xFF, 0x01);
    if (status != VL53L0X_ERROR_NONE) return status;
    status = VL53L0X_WriteByte(Dev, 0x4E, 0x2C);
    if (status != VL53L0X_ERROR_NONE) return status;
    status = VL53L0X_WriteByte(Dev, 0x48, 0x00);
    if (status != VL53L0X_ERROR_NONE) return status;
    status = VL53L0X_WriteByte(Dev, 0x30, 0x20);
    if (status != VL53L0X_ERROR_NONE) return status;
    
    status = VL53L0X_WriteByte(Dev, 0xFF, 0x00);
    if (status != VL53L0X_ERROR_NONE) return status;
    status = VL53L0X_WriteByte(Dev, 0x30, 0x09);
    if (status != VL53L0X_ERROR_NONE) return status;
    status = VL53L0X_WriteByte(Dev, 0x54, 0x00);
    if (status != VL53L0X_ERROR_NONE) return status;
    status = VL53L0X_WriteByte(Dev, 0x31, 0x04);
    if (status != VL53L0X_ERROR_NONE) return status;
    status = VL53L0X_WriteByte(Dev, 0x32, 0x03);
    if (status != VL53L0X_ERROR_NONE) return status;
    status = VL53L0X_WriteByte(Dev, 0x40, 0x83);
    if (status != VL53L0X_ERROR_NONE) return status;
    status = VL53L0X_WriteByte(Dev, 0x46, 0x25);
    if (status != VL53L0X_ERROR_NONE) return status;
    status = VL53L0X_WriteByte(Dev, 0x60, 0x00);
    if (status != VL53L0X_ERROR_NONE) return status;
    status = VL53L0X_WriteByte(Dev, 0x27, 0x00);
    if (status != VL53L0X_ERROR_NONE) return status;
    status = VL53L0X_WriteByte(Dev, 0x50, 0x06);
    if (status != VL53L0X_ERROR_NONE) return status;
    status = VL53L0X_WriteByte(Dev, 0x51, 0x00);
    if (status != VL53L0X_ERROR_NONE) return status;
    status = VL53L0X_WriteByte(Dev, 0x52, 0x96);
    if (status != VL53L0X_ERROR_NONE) return status;
    status = VL53L0X_WriteByte(Dev, 0x56, 0x08);
    if (status != VL53L0X_ERROR_NONE) return status;
    status = VL53L0X_WriteByte(Dev, 0x57, 0x30);
    if (status != VL53L0X_ERROR_NONE) return status;
    status = VL53L0X_WriteByte(Dev, 0x61, 0x00);
    if (status != VL53L0X_ERROR_NONE) return status;
    status = VL53L0X_WriteByte(Dev, 0x62, 0x00);
    if (status != VL53L0X_ERROR_NONE) return status;
    status = VL53L0X_WriteByte(Dev, 0x64, 0x00);
    if (status != VL53L0X_ERROR_NONE) return status;
    status = VL53L0X_WriteByte(Dev, 0x65, 0x00);
    if (status != VL53L0X_ERROR_NONE) return status;
    status = VL53L0X_WriteByte(Dev, 0x66, 0xA0);
    if (status != VL53L0X_ERROR_NONE) return status;
    
    status = VL53L0X_WriteByte(Dev, 0xFF, 0x01);
    if (status != VL53L0X_ERROR_NONE) return status;
    status = VL53L0X_WriteByte(Dev, 0x22, 0x32);
    if (status != VL53L0X_ERROR_NONE) return status;
    status = VL53L0X_WriteByte(Dev, 0x47, 0x14);
    if (status != VL53L0X_ERROR_NONE) return status;
    status = VL53L0X_WriteByte(Dev, 0x49, 0xFF);
    if (status != VL53L0X_ERROR_NONE) return status;
    status = VL53L0X_WriteByte(Dev, 0x4A, 0x00);
    if (status != VL53L0X_ERROR_NONE) return status;
    
    status = VL53L0X_WriteByte(Dev, 0xFF, 0x00);
    if (status != VL53L0X_ERROR_NONE) return status;
    status = VL53L0X_WriteByte(Dev, 0x7A, 0x0A);
    if (status != VL53L0X_ERROR_NONE) return status;
    status = VL53L0X_WriteByte(Dev, 0x7B, 0x00);
    if (status != VL53L0X_ERROR_NONE) return status;
    status = VL53L0X_WriteByte(Dev, 0x78, 0x21);
    if (status != VL53L0X_ERROR_NONE) return status;
    
    status = VL53L0X_WriteByte(Dev, 0xFF, 0x01);
    if (status != VL53L0X_ERROR_NONE) return status;
    status = VL53L0X_WriteByte(Dev, 0x23, 0x34);
    if (status != VL53L0X_ERROR_NONE) return status;
    status = VL53L0X_WriteByte(Dev, 0x42, 0x00);
    if (status != VL53L0X_ERROR_NONE) return status;
    status = VL53L0X_WriteByte(Dev, 0x44, 0xFF);
    if (status != VL53L0X_ERROR_NONE) return status;
    status = VL53L0X_WriteByte(Dev, 0x45, 0x26);
    if (status != VL53L0X_ERROR_NONE) return status;
    status = VL53L0X_WriteByte(Dev, 0x46, 0x05);
    if (status != VL53L0X_ERROR_NONE) return status;
    status = VL53L0X_WriteByte(Dev, 0x40, 0x40);
    if (status != VL53L0X_ERROR_NONE) return status;
    status = VL53L0X_WriteByte(Dev, 0x0E, 0x06);
    if (status != VL53L0X_ERROR_NONE) return status;
    status = VL53L0X_WriteByte(Dev, 0x20, 0x1A);
    if (status != VL53L0X_ERROR_NONE) return status;
    status = VL53L0X_WriteByte(Dev, 0x43, 0x40);
    if (status != VL53L0X_ERROR_NONE) return status;
    
    status = VL53L0X_WriteByte(Dev, 0xFF, 0x00);
    if (status != VL53L0X_ERROR_NONE) return status;
    status = VL53L0X_WriteByte(Dev, 0x34, 0x03);
    if (status != VL53L0X_ERROR_NONE) return status;
    status = VL53L0X_WriteByte(Dev, 0x35, 0x44);
    if (status != VL53L0X_ERROR_NONE) return status;
    
    status = VL53L0X_WriteByte(Dev, 0xFF, 0x01);
    if (status != VL53L0X_ERROR_NONE) return status;
    status = VL53L0X_WriteByte(Dev, 0x31, 0x04);
    if (status != VL53L0X_ERROR_NONE) return status;
    status = VL53L0X_WriteByte(Dev, 0x4B, 0x09);
    if (status != VL53L0X_ERROR_NONE) return status;
    status = VL53L0X_WriteByte(Dev, 0x4C, 0x05);
    if (status != VL53L0X_ERROR_NONE) return status;
    status = VL53L0X_WriteByte(Dev, 0x4D, 0x04);
    if (status != VL53L0X_ERROR_NONE) return status;
    
    status = VL53L0X_WriteByte(Dev, 0xFF, 0x00);
    if (status != VL53L0X_ERROR_NONE) return status;
    status = VL53L0X_WriteByte(Dev, 0x44, 0x00);
    if (status != VL53L0X_ERROR_NONE) return status;
    status = VL53L0X_WriteByte(Dev, 0x45, 0x20);
    if (status != VL53L0X_ERROR_NONE) return status;
    status = VL53L0X_WriteByte(Dev, 0x47, 0x08);
    if (status != VL53L0X_ERROR_NONE) return status;
    status = VL53L0X_WriteByte(Dev, 0x48, 0x28);
    if (status != VL53L0X_ERROR_NONE) return status;
    status = VL53L0X_WriteByte(Dev, 0x67, 0x00);
    if (status != VL53L0X_ERROR_NONE) return status;
    status = VL53L0X_WriteByte(Dev, 0x70, 0x04);
    if (status != VL53L0X_ERROR_NONE) return status;
    status = VL53L0X_WriteByte(Dev, 0x71, 0x01);
    if (status != VL53L0X_ERROR_NONE) return status;
    status = VL53L0X_WriteByte(Dev, 0x72, 0xFE);
    if (status != VL53L0X_ERROR_NONE) return status;
    status = VL53L0X_WriteByte(Dev, 0x76, 0x00);
    if (status != VL53L0X_ERROR_NONE) return status;
    status = VL53L0X_WriteByte(Dev, 0x77, 0x00);
    if (status != VL53L0X_ERROR_NONE) return status;
    
    status = VL53L0X_WriteByte(Dev, 0xFF, 0x01);
    if (status != VL53L0X_ERROR_NONE) return status;
    status = VL53L0X_WriteByte(Dev, 0x0D, 0x01);
    if (status != VL53L0X_ERROR_NONE) return status;
    
    status = VL53L0X_WriteByte(Dev, 0xFF, 0x00);
    if (status != VL53L0X_ERROR_NONE) return status;
    status = VL53L0X_WriteByte(Dev, 0x80, 0x01);
    if (status != VL53L0X_ERROR_NONE) return status;
    status = VL53L0X_WriteByte(Dev, 0x01, 0xF8);
    if (status != VL53L0X_ERROR_NONE) return status;
    
    status = VL53L0X_WriteByte(Dev, 0xFF, 0x01);
    if (status != VL53L0X_ERROR_NONE) return status;
    status = VL53L0X_WriteByte(Dev, 0x8E, 0x01);
    if (status != VL53L0X_ERROR_NONE) return status;
    status = VL53L0X_WriteByte(Dev, 0x00, 0x01);
    if (status != VL53L0X_ERROR_NONE) return status;
    status = VL53L0X_WriteByte(Dev, 0xFF, 0x00);
    if (status != VL53L0X_ERROR_NONE) return status;
    status = VL53L0X_WriteByte(Dev, 0x80, 0x00);
    if (status != VL53L0X_ERROR_NONE) return status;
    
    // Configure GPIO interrupt for new sample ready (CRITICAL!)
    status = VL53L0X_WriteByte(Dev, VL53L0X_REG_SYSTEM_INTERRUPT_CONFIG_GPIO, 0x04);
    if (status != VL53L0X_ERROR_NONE) return status;
    
    // Set GPIO HV MUX to active low
    uint8_t reg_val;
    status = VL53L0X_ReadByte(Dev, 0x84, &reg_val);
    if (status != VL53L0X_ERROR_NONE) return status;
    status = VL53L0X_WriteByte(Dev, 0x84, reg_val & ~0x10);
    if (status != VL53L0X_ERROR_NONE) return status;
    
    // Clear any pending interrupts
    status = VL53L0X_WriteByte(Dev, VL53L0X_REG_SYSTEM_INTERRUPT_CLEAR, 0x01);
    if (status != VL53L0X_ERROR_NONE) return status;
    
    // Set sequence config (MSRC+TCC disabled)
    status = VL53L0X_WriteByte(Dev, VL53L0X_REG_SYSTEM_SEQUENCE_CONFIG, 0xE8);
    if (status != VL53L0X_ERROR_NONE) return status;
    
    ESP_LOGI(TAG, "Static initialization complete (with GPIO interrupt config)");
    
    return VL53L0X_ERROR_NONE;
}

VL53L0X_Error VL53L0X_GetDeviceInfo(VL53L0X_DEV Dev, VL53L0X_DeviceInfo_t *pVL53L0X_DeviceInfo) {
    VL53L0X_Error status = VL53L0X_ERROR_NONE;
    uint8_t model_id = 0;
    uint8_t revision_id = 0;
    
    status = VL53L0X_ReadByte(Dev, VL53L0X_REG_IDENTIFICATION_MODEL_ID, &model_id);
    if (status != VL53L0X_ERROR_NONE) return status;
    
    status = VL53L0X_ReadByte(Dev, VL53L0X_REG_IDENTIFICATION_REVISION_ID, &revision_id);
    if (status != VL53L0X_ERROR_NONE) return status;
    
    strncpy(pVL53L0X_DeviceInfo->Name, "VL53L0X", sizeof(pVL53L0X_DeviceInfo->Name) - 1);
    strncpy(pVL53L0X_DeviceInfo->Type, "VL53L0X", sizeof(pVL53L0X_DeviceInfo->Type) - 1);
    snprintf(pVL53L0X_DeviceInfo->ProductId, sizeof(pVL53L0X_DeviceInfo->ProductId), "0x%02X", model_id);
    pVL53L0X_DeviceInfo->ProductType = model_id;
    pVL53L0X_DeviceInfo->ProductRevisionMajor = (revision_id >> 4) & 0x0F;
    pVL53L0X_DeviceInfo->ProductRevisionMinor = revision_id & 0x0F;
    
    return VL53L0X_ERROR_NONE;
}

// Helper function matching Pololu's performSingleRefCalibration
static VL53L0X_Error VL53L0X_PerformSingleRefCalibration(VL53L0X_DEV Dev, uint8_t vhv_init_byte) {
    VL53L0X_Error status = VL53L0X_ERROR_NONE;
    uint8_t interrupt_status = 0;
    uint16_t timeout = 0;
    
    // Start calibration with vhv_init_byte (0x40 for VHV, 0x00 for Phase)
    status = VL53L0X_WriteByte(Dev, VL53L0X_REG_SYSRANGE_START, 0x01 | vhv_init_byte);
    if (status != VL53L0X_ERROR_NONE) return status;
    
    // Wait for calibration to complete - check bits 0-2 (Pololu way)
    do {
        status = VL53L0X_ReadByte(Dev, VL53L0X_REG_RESULT_INTERRUPT_STATUS, &interrupt_status);
        if (status != VL53L0X_ERROR_NONE) return status;
        
        if (timeout++ > 1000) {
            ESP_LOGE(TAG, "Single ref calibration timeout (vhv_byte=0x%02X)", vhv_init_byte);
            return VL53L0X_ERROR_TIME_OUT;
        }
        VL53L0X_PollingDelay(Dev, 1);
    } while ((interrupt_status & 0x07) == 0);
    
    // Clear interrupt
    status = VL53L0X_WriteByte(Dev, VL53L0X_REG_SYSTEM_INTERRUPT_CLEAR, 0x01);
    if (status != VL53L0X_ERROR_NONE) return status;
    
    // Stop
    status = VL53L0X_WriteByte(Dev, VL53L0X_REG_SYSRANGE_START, 0x00);
    
    return status;
}

VL53L0X_Error VL53L0X_PerformRefCalibration(VL53L0X_DEV Dev, uint8_t *VhvSettings, uint8_t *PhaseCal) {
    VL53L0X_Error status = VL53L0X_ERROR_NONE;
    
    ESP_LOGI(TAG, "Performing reference calibration (Pololu method)...");
    
    // VHV calibration
    status = VL53L0X_WriteByte(Dev, VL53L0X_REG_SYSTEM_SEQUENCE_CONFIG, 0x01);
    if (status != VL53L0X_ERROR_NONE) return status;
    
    status = VL53L0X_PerformSingleRefCalibration(Dev, 0x40);
    if (status != VL53L0X_ERROR_NONE) {
        ESP_LOGE(TAG, "VHV calibration failed");
        return status;
    }
    uint8_t sequence_config = 0;
    
    // Read VHV settings
    status = VL53L0X_ReadByte(Dev, 0xCB, VhvSettings);
    if (status != VL53L0X_ERROR_NONE) return status;
    
    ESP_LOGI(TAG, "VHV calibration complete (VHV=0x%02X)", *VhvSettings);
    
    // Phase calibration
    status = VL53L0X_WriteByte(Dev, VL53L0X_REG_SYSTEM_SEQUENCE_CONFIG, 0x02);
    if (status != VL53L0X_ERROR_NONE) return status;
    
    status = VL53L0X_PerformSingleRefCalibration(Dev, 0x00);
    if (status != VL53L0X_ERROR_NONE) {
        ESP_LOGE(TAG, "Phase calibration failed");
        return status;
    }
    
    // Read Phase settings
    status = VL53L0X_ReadByte(Dev, 0xEE, PhaseCal);
    if (status != VL53L0X_ERROR_NONE) return status;
    
    // Restore sequence config to 0xE8 (MSRC+TCC disabled, Final Range enabled)
    status = VL53L0X_WriteByte(Dev, VL53L0X_REG_SYSTEM_SEQUENCE_CONFIG, 0xE8);
    if (status != VL53L0X_ERROR_NONE) return status;
    
    // Configure GPIO interrupt for measurements
    status = VL53L0X_WriteByte(Dev, VL53L0X_REG_SYSTEM_INTERRUPT_CONFIG_GPIO, 0x04);
    if (status != VL53L0X_ERROR_NONE) return status;
    
    // Clear any pending interrupts
    status = VL53L0X_WriteByte(Dev, VL53L0X_REG_SYSTEM_INTERRUPT_CLEAR, 0x01);
    if (status != VL53L0X_ERROR_NONE) return status;
    
    ESP_LOGI(TAG, "Reference calibration complete (VHV=0x%02X, Phase=0x%02X)", *VhvSettings, *PhaseCal);
    
    return VL53L0X_ERROR_NONE;
}

VL53L0X_Error VL53L0X_PerformRefSpadManagement(VL53L0X_DEV Dev, uint32_t *refSpadCount, uint8_t *isApertureSpads) {
    // Simplified implementation - in production, this should be more comprehensive
    *refSpadCount = 44;
    *isApertureSpads = 1;
    
    ESP_LOGI(TAG, "SPAD management: count=%lu, aperture=%d", *refSpadCount, *isApertureSpads);
    
    return VL53L0X_ERROR_NONE;
}

VL53L0X_Error VL53L0X_SetDeviceMode(VL53L0X_DEV Dev, VL53L0X_DeviceModes DeviceMode) {
    if (Dev == NULL) {
        return VL53L0X_ERROR_INVALID_PARAMS;
    }
    
    Dev->CurrentParameters.DeviceMode = DeviceMode;
    ESP_LOGI(TAG, "Device mode set to %d", DeviceMode);
    
    return VL53L0X_ERROR_NONE;
}

VL53L0X_Error VL53L0X_GetDeviceMode(VL53L0X_DEV Dev, VL53L0X_DeviceModes *pDeviceMode) {
    if (Dev == NULL || pDeviceMode == NULL) {
        return VL53L0X_ERROR_INVALID_PARAMS;
    }
    
    *pDeviceMode = Dev->CurrentParameters.DeviceMode;
    
    return VL53L0X_ERROR_NONE;
}

VL53L0X_Error VL53L0X_StartMeasurement(VL53L0X_DEV Dev) {
    VL53L0X_Error status = VL53L0X_ERROR_NONE;
    uint8_t device_mode;
    
    device_mode = Dev->CurrentParameters.DeviceMode;
    
    status = VL53L0X_WriteByte(Dev, 0x80, 0x01);
    if (status != VL53L0X_ERROR_NONE) return status;
    
    status = VL53L0X_WriteByte(Dev, 0xFF, 0x01);
    if (status != VL53L0X_ERROR_NONE) return status;
    
    status = VL53L0X_WriteByte(Dev, 0x00, 0x00);
    if (status != VL53L0X_ERROR_NONE) return status;
    
    status = VL53L0X_WriteByte(Dev, 0x91, 0x3C);
    if (status != VL53L0X_ERROR_NONE) return status;
    
    status = VL53L0X_WriteByte(Dev, 0x00, 0x01);
    if (status != VL53L0X_ERROR_NONE) return status;
    
    status = VL53L0X_WriteByte(Dev, 0xFF, 0x00);
    if (status != VL53L0X_ERROR_NONE) return status;
    
    status = VL53L0X_WriteByte(Dev, 0x80, 0x00);
    if (status != VL53L0X_ERROR_NONE) return status;
    
    if (device_mode == VL53L0X_DEVICEMODE_SINGLE_RANGING) {
        status = VL53L0X_WriteByte(Dev, VL53L0X_REG_SYSRANGE_START, 0x01);
    } else if (device_mode == VL53L0X_DEVICEMODE_CONTINUOUS_RANGING) {
        status = VL53L0X_WriteByte(Dev, VL53L0X_REG_SYSRANGE_START, 0x02);
        if (status == VL53L0X_ERROR_NONE) {
            // Allow time for continuous mode to initialize
            VL53L0X_PollingDelay(Dev, 10);
        }
    } else {
        status = VL53L0X_ERROR_MODE_NOT_SUPPORTED;
    }
    
    return status;
}

VL53L0X_Error VL53L0X_StopMeasurement(VL53L0X_DEV Dev) {
    VL53L0X_Error status = VL53L0X_ERROR_NONE;
    
    status = VL53L0X_WriteByte(Dev, VL53L0X_REG_SYSRANGE_START, 0x00);
    if (status != VL53L0X_ERROR_NONE) return status;
    
    status = VL53L0X_WriteByte(Dev, 0xFF, 0x01);
    if (status != VL53L0X_ERROR_NONE) return status;
    
    status = VL53L0X_WriteByte(Dev, 0x00, 0x00);
    if (status != VL53L0X_ERROR_NONE) return status;
    
    status = VL53L0X_WriteByte(Dev, 0x91, 0x00);
    if (status != VL53L0X_ERROR_NONE) return status;
    
    status = VL53L0X_WriteByte(Dev, 0x00, 0x01);
    if (status != VL53L0X_ERROR_NONE) return status;
    
    status = VL53L0X_WriteByte(Dev, 0xFF, 0x00);
    if (status != VL53L0X_ERROR_NONE) return status;
    
    return VL53L0X_ERROR_NONE;
}

VL53L0X_Error VL53L0X_GetRangingMeasurementData(VL53L0X_DEV Dev, VL53L0X_RangingMeasurementData_t *pRangingMeasurementData) {
    VL53L0X_Error status = VL53L0X_ERROR_NONE;
    uint8_t buffer[12];
    
    status = VL53L0X_ReadMulti(Dev, VL53L0X_REG_RESULT_RANGE_STATUS, buffer, 12);
    if (status != VL53L0X_ERROR_NONE) return status;
    
    pRangingMeasurementData->RangeStatus = (buffer[0] >> 3) & 0x07;
    pRangingMeasurementData->RangeMilliMeter = ((uint16_t)buffer[10] << 8) | buffer[11];
    
    pRangingMeasurementData->SignalRateRtnMegaCps = (((uint32_t)buffer[6] << 8) | buffer[7]);
    pRangingMeasurementData->AmbientRateRtnMegaCps = (((uint32_t)buffer[8] << 8) | buffer[9]);
    pRangingMeasurementData->EffectiveSpadRtnCount = (((uint16_t)buffer[2] << 8) | buffer[3]);
    
    VL53L0X_GetTickCount(&pRangingMeasurementData->TimeStamp);
    
    return VL53L0X_ERROR_NONE;
}

VL53L0X_Error VL53L0X_PerformSingleRangingMeasurement(VL53L0X_DEV Dev, VL53L0X_RangingMeasurementData_t *pRangingMeasurementData) {
    VL53L0X_Error status = VL53L0X_ERROR_NONE;
    
    status = VL53L0X_SetDeviceMode(Dev, VL53L0X_DEVICEMODE_SINGLE_RANGING);
    if (status != VL53L0X_ERROR_NONE) return status;
    
    status = VL53L0X_StartMeasurement(Dev);
    if (status != VL53L0X_ERROR_NONE) return status;
    
    // Wait for measurement ready
    uint8_t interrupt_status = 0;
    uint16_t timeout = 0;
    do {
        status = VL53L0X_ReadByte(Dev, VL53L0X_REG_RESULT_INTERRUPT_STATUS, &interrupt_status);
        if (status != VL53L0X_ERROR_NONE) return status;
        
        if (timeout++ > 1000) {
            ESP_LOGE(TAG, "Measurement timeout");
            return VL53L0X_ERROR_TIME_OUT;
        }
        VL53L0X_PollingDelay(Dev, 1);
    } while ((interrupt_status & 0x07) == 0);
    
    status = VL53L0X_GetRangingMeasurementData(Dev, pRangingMeasurementData);
    if (status != VL53L0X_ERROR_NONE) return status;
    
    status = VL53L0X_ClearInterruptMask(Dev, 0);
    if (status != VL53L0X_ERROR_NONE) return status;
    
    return VL53L0X_ERROR_NONE;
}

VL53L0X_Error VL53L0X_ClearInterruptMask(VL53L0X_DEV Dev, uint32_t InterruptMask) {
    VL53L0X_Error status = VL53L0X_ERROR_NONE;
    
    // Only clear interrupt - do NOT stop continuous mode
    status = VL53L0X_WriteByte(Dev, VL53L0X_REG_SYSTEM_INTERRUPT_CLEAR, 0x01);
    
    return status;
}

VL53L0X_Error VL53L0X_GetInterruptMaskStatus(VL53L0X_DEV Dev, uint32_t *pInterruptMaskStatus) {
    VL53L0X_Error status = VL53L0X_ERROR_NONE;
    uint8_t byte;
    
    status = VL53L0X_ReadByte(Dev, VL53L0X_REG_RESULT_INTERRUPT_STATUS, &byte);
    // Use bits 0-2 matching Pololu implementation
    *pInterruptMaskStatus = byte & 0x07;
    
    return status;
}

VL53L0X_Error VL53L0X_SetMeasurementTimingBudgetMicroSeconds(VL53L0X_DEV Dev, uint32_t MeasurementTimingBudgetMicroSeconds) {
    if (Dev == NULL) {
        return VL53L0X_ERROR_INVALID_PARAMS;
    }
    
    Dev->CurrentParameters.MeasurementTimingBudgetMicroSeconds = MeasurementTimingBudgetMicroSeconds;
    ESP_LOGI(TAG, "Measurement timing budget set to %lu us", MeasurementTimingBudgetMicroSeconds);
    
    return VL53L0X_ERROR_NONE;
}

VL53L0X_Error VL53L0X_GetMeasurementTimingBudgetMicroSeconds(VL53L0X_DEV Dev, uint32_t *pMeasurementTimingBudgetMicroSeconds) {
    if (Dev == NULL || pMeasurementTimingBudgetMicroSeconds == NULL) {
        return VL53L0X_ERROR_INVALID_PARAMS;
    }
    
    *pMeasurementTimingBudgetMicroSeconds = Dev->CurrentParameters.MeasurementTimingBudgetMicroSeconds;
    
    return VL53L0X_ERROR_NONE;
}

VL53L0X_Error VL53L0X_SetInterMeasurementPeriodMilliSeconds(VL53L0X_DEV Dev, uint32_t InterMeasurementPeriodMilliSeconds) {
    VL53L0X_Error status = VL53L0X_ERROR_NONE;
    
    if (Dev == NULL) {
        return VL53L0X_ERROR_INVALID_PARAMS;
    }
    
    uint16_t osc_calibrate_val = 0;
    uint32_t inter_measurement_period_mclks;
    
    status = VL53L0X_ReadWord(Dev, 0xF8, &osc_calibrate_val);
    if (status != VL53L0X_ERROR_NONE) return status;
    
    inter_measurement_period_mclks = InterMeasurementPeriodMilliSeconds * osc_calibrate_val;
    
    status = VL53L0X_WriteDWord(Dev, VL53L0X_REG_SYSTEM_INTERMEASUREMENT_PERIOD, inter_measurement_period_mclks);
    if (status != VL53L0X_ERROR_NONE) return status;
    
    Dev->CurrentParameters.InterMeasurementPeriodMilliSeconds = InterMeasurementPeriodMilliSeconds;
    
    ESP_LOGI(TAG, "Inter-measurement period set to %lu ms", InterMeasurementPeriodMilliSeconds);
    
    return VL53L0X_ERROR_NONE;
}

VL53L0X_Error VL53L0X_GetInterMeasurementPeriodMilliSeconds(VL53L0X_DEV Dev, uint32_t *pInterMeasurementPeriodMilliSeconds) {
    if (Dev == NULL || pInterMeasurementPeriodMilliSeconds == NULL) {
        return VL53L0X_ERROR_INVALID_PARAMS;
    }
    
    *pInterMeasurementPeriodMilliSeconds = Dev->CurrentParameters.InterMeasurementPeriodMilliSeconds;
    
    return VL53L0X_ERROR_NONE;
}

VL53L0X_Error VL53L0X_SetLimitCheckEnable(VL53L0X_DEV Dev, uint16_t LimitCheckId, uint8_t LimitCheckEnable) {
    // Simplified implementation
    ESP_LOGI(TAG, "Limit check %d set to %d", LimitCheckId, LimitCheckEnable);
    return VL53L0X_ERROR_NONE;
}

VL53L0X_Error VL53L0X_SetLimitCheckValue(VL53L0X_DEV Dev, uint16_t LimitCheckId, uint32_t LimitCheckValue) {
    // Simplified implementation
    ESP_LOGI(TAG, "Limit check %d value set to %lu", LimitCheckId, LimitCheckValue);
    return VL53L0X_ERROR_NONE;
}

// Mode Configuration Functions

VL53L0X_Error VL53L0X_SetHighAccuracyMode(VL53L0X_DEV Dev) {
    VL53L0X_Error status = VL53L0X_ERROR_NONE;
    
    ESP_LOGI(TAG, "Setting High Accuracy Mode");
    
    status = VL53L0X_SetMeasurementTimingBudgetMicroSeconds(Dev, 200000);  // 200ms
    if (status != VL53L0X_ERROR_NONE) return status;
    
    status = VL53L0X_WriteByte(Dev, VL53L0X_REG_FINAL_RANGE_CONFIG_MIN_COUNT_RATE_RTN_LIMIT, 0x00);
    if (status != VL53L0X_ERROR_NONE) return status;
    
    return VL53L0X_ERROR_NONE;
}

VL53L0X_Error VL53L0X_SetLongRangeMode(VL53L0X_DEV Dev) {
    VL53L0X_Error status = VL53L0X_ERROR_NONE;
    
    ESP_LOGI(TAG, "Setting Long Range Mode");
    
    status = VL53L0X_SetMeasurementTimingBudgetMicroSeconds(Dev, 33000);  // 33ms
    if (status != VL53L0X_ERROR_NONE) return status;
    
    status = VL53L0X_WriteByte(Dev, VL53L0X_REG_FINAL_RANGE_CONFIG_MIN_COUNT_RATE_RTN_LIMIT, 0x19);
    if (status != VL53L0X_ERROR_NONE) return status;
    
    status = VL53L0X_WriteByte(Dev, VL53L0X_REG_FINAL_RANGE_CONFIG_VCSEL_PERIOD, 0x0E);
    if (status != VL53L0X_ERROR_NONE) return status;
    
    status = VL53L0X_WriteByte(Dev, VL53L0X_REG_PRE_RANGE_CONFIG_VCSEL_PERIOD, 0x0E);
    if (status != VL53L0X_ERROR_NONE) return status;
    
    return VL53L0X_ERROR_NONE;
}

VL53L0X_Error VL53L0X_SetHighSpeedMode(VL53L0X_DEV Dev) {
    VL53L0X_Error status = VL53L0X_ERROR_NONE;
    
    ESP_LOGI(TAG, "Setting High Speed Mode");
    
    status = VL53L0X_SetMeasurementTimingBudgetMicroSeconds(Dev, 20000);  // 20ms
    if (status != VL53L0X_ERROR_NONE) return status;
    
    status = VL53L0X_WriteByte(Dev, VL53L0X_REG_FINAL_RANGE_CONFIG_MIN_COUNT_RATE_RTN_LIMIT, 0x60);
    if (status != VL53L0X_ERROR_NONE) return status;
    
    return VL53L0X_ERROR_NONE;
}

VL53L0X_Error VL53L0X_SetDefaultMode(VL53L0X_DEV Dev) {
    VL53L0X_Error status = VL53L0X_ERROR_NONE;
    
    ESP_LOGI(TAG, "Setting Default Mode");
    
    status = VL53L0X_SetMeasurementTimingBudgetMicroSeconds(Dev, 33000);  // 33ms
    if (status != VL53L0X_ERROR_NONE) return status;
    
    status = VL53L0X_WriteByte(Dev, VL53L0X_REG_FINAL_RANGE_CONFIG_MIN_COUNT_RATE_RTN_LIMIT, 0x20);
    if (status != VL53L0X_ERROR_NONE) return status;
    
    return VL53L0X_ERROR_NONE;
}

// Multi-Sensor Support Functions

VL53L0X_Error VL53L0X_SetDeviceAddress(VL53L0X_DEV Dev, uint8_t new_address) {
    VL53L0X_Error status = VL53L0X_ERROR_NONE;
    
    if (Dev == NULL) {
        return VL53L0X_ERROR_INVALID_PARAMS;
    }
    
    if (new_address == Dev->DeviceSpecificParameters.I2cDevAddr) {
        ESP_LOGW(TAG, "New address 0x%02X is same as current address", new_address);
        return VL53L0X_ERROR_NONE;
    }
    
    // Write new I2C address to device
    status = VL53L0X_WriteByte(Dev, VL53L0X_REG_I2C_SLAVE_DEVICE_ADDRESS, new_address);
    if (status != VL53L0X_ERROR_NONE) {
        ESP_LOGE(TAG, "Failed to write new I2C address");
        return status;
    }
    
    // Update device structure
    Dev->DeviceSpecificParameters.I2cDevAddr = new_address;
    
    ESP_LOGI(TAG, "I2C address changed to 0x%02X", new_address);
    
    return VL53L0X_ERROR_NONE;
}

VL53L0X_Error VL53L0X_InitXShutPin(int xshut_pin) {
    if (xshut_pin < 0 || xshut_pin > 39) {
        ESP_LOGE(TAG, "Invalid XSHUT pin: %d", xshut_pin);
        return VL53L0X_ERROR_INVALID_PARAMS;
    }
    
    // Configure GPIO as output
    gpio_config_t io_conf = {
        .pin_bit_mask = (1ULL << xshut_pin),
        .mode = GPIO_MODE_OUTPUT,
        .pull_up_en = GPIO_PULLUP_DISABLE,
        .pull_down_en = GPIO_PULLDOWN_DISABLE,
        .intr_type = GPIO_INTR_DISABLE
    };
    
    esp_err_t err = gpio_config(&io_conf);
    if (err != ESP_OK) {
        ESP_LOGE(TAG, "Failed to configure XSHUT pin %d: %d", xshut_pin, err);
        return VL53L0X_ERROR_GPIO_NOT_EXISTING;
    }
    
    // Set pin LOW initially (sensor disabled)
    gpio_set_level(xshut_pin, 0);
    
    ESP_LOGI(TAG, "XSHUT pin %d initialized (sensor disabled)", xshut_pin);
    
    return VL53L0X_ERROR_NONE;
}

VL53L0X_Error VL53L0X_SetXShutState(int xshut_pin, bool enable) {
    if (xshut_pin < 0 || xshut_pin > 39) {
        ESP_LOGE(TAG, "Invalid XSHUT pin: %d", xshut_pin);
        return VL53L0X_ERROR_INVALID_PARAMS;
    }
    
    esp_err_t err = gpio_set_level(xshut_pin, enable ? 1 : 0);
    if (err != ESP_OK) {
        ESP_LOGE(TAG, "Failed to set XSHUT pin %d: %d", xshut_pin, err);
        return VL53L0X_ERROR_GPIO_FUNCTIONALITY_NOT_SUPPORTED;
    }
    
    ESP_LOGD(TAG, "XSHUT pin %d set to %s", xshut_pin, enable ? "HIGH (enabled)" : "LOW (disabled)");
    
    return VL53L0X_ERROR_NONE;
}
