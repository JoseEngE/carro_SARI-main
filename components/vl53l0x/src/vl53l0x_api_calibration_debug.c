/*
 * VL53L0X Calibration Debug - Alternative implementation with detailed logging
 * Use this to diagnose calibration issues
 */

#include "vl53l0x.h"
#include "esp_log.h"

static const char *TAG = "VL53L0X_CAL_DEBUG";

// Register definitions
#define VL53L0X_REG_SYSRANGE_START                  0x00
#define VL53L0X_REG_SYSTEM_SEQUENCE_CONFIG          0x01
#define VL53L0X_REG_SYSTEM_INTERRUPT_CONFIG_GPIO    0x0A
#define VL53L0X_REG_SYSTEM_INTERRUPT_CLEAR          0x0B
#define VL53L0X_REG_RESULT_INTERRUPT_STATUS         0x13
#define VL53L0X_REG_RESULT_RANGE_STATUS             0x14

/**
 * @brief Alternative VHV calibration with detailed debugging
 */
VL53L0X_Error VL53L0X_PerformRefCalibration_Debug(VL53L0X_DEV Dev, uint8_t *VhvSettings, uint8_t *PhaseCal) {
    VL53L0X_Error status = VL53L0X_ERROR_NONE;
    uint8_t sysrange_start = 0;
    uint8_t system_sequence_config = 0;
    
    ESP_LOGI(TAG, "═══════════════════════════════════════════════");
    ESP_LOGI(TAG, "  Starting DETAILED Calibration Diagnostic");
    ESP_LOGI(TAG, "═══════════════════════════════════════════════");
    
    // Step 1: Read current state before calibration
    ESP_LOGI(TAG, "[PRE-CAL] Reading current register states...");
    
    status = VL53L0X_ReadByte(Dev, VL53L0X_REG_SYSRANGE_START, &sysrange_start);
    if (status != VL53L0X_ERROR_NONE) {
        ESP_LOGE(TAG, "[PRE-CAL] Failed to read SYSRANGE_START: %d", status);
        return status;
    }
    ESP_LOGI(TAG, "[PRE-CAL] SYSRANGE_START = 0x%02X", sysrange_start);
    
    status = VL53L0X_ReadByte(Dev,VL53L0X_REG_SYSTEM_SEQUENCE_CONFIG, &system_sequence_config);
    if (status != VL53L0X_ERROR_NONE) {
        ESP_LOGE(TAG, "[PRE-CAL] Failed to read SEQUENCE_CONFIG: %d", status);
        return status;
    }
    ESP_LOGI(TAG, "[PRE-CAL] SEQUENCE_CONFIG = 0x%02X", system_sequence_config);
    
    // Step 2: Configure for VHV calibration
    ESP_LOGI(TAG, "[VHV] Configuring VHV calibration sequence...");
    
    status = VL53L0X_WriteByte(Dev, VL53L0X_REG_SYSTEM_SEQUENCE_CONFIG, 0x01);
    if (status != VL53L0X_ERROR_NONE) {
        ESP_LOGE(TAG, "[VHV] Failed to set SEQUENCE_CONFIG to 0x01: %d", status);
        return status;
    }
    ESP_LOGI(TAG, "[VHV] ✓ SEQUENCE_CONFIG set to 0x01 (VHV only)");
    
    // Delay to allow sensor to process
    VL53L0X_PollingDelay(Dev, 5);
    
    // Step 3: Start VHV calibration
    ESP_LOGI(TAG, "[VHV] Starting VHV calibration...");
    
    // VHV: write 0x01 | 0x40 = 0x41 to SYSRANGE_START (Pololu implementation)
    status = VL53L0X_WriteByte(Dev, VL53L0X_REG_SYSRANGE_START, 0x01 | 0x40);
    if (status != VL53L0X_ERROR_NONE) {
        ESP_LOGE(TAG, "[VHV] Failed to write SYSRANGE_START: %d", status);
        return status;
    }
    ESP_LOGI(TAG, "[VHV] ✓ SYSRANGE_START written as 0x41 (0x01|0x40)");
    
    // Step 4: Wait and monitor
    ESP_LOGI(TAG, "[VHV] Waiting for calibration to complete...");
    VL53L0X_PollingDelay(Dev, 10);
    
    // Step 5: Poll for completion with detailed logging
    uint8_t interrupt_status = 0;
    uint8_t range_status = 0;
    uint16_t timeout = 0;
    bool first_log = true;
    
    ESP_LOGI(TAG, "[VHV] Polling interrupt status...");
    
    do {
        status = VL53L0X_ReadByte(Dev, VL53L0X_REG_RESULT_INTERRUPT_STATUS, &interrupt_status);
        if (status != VL53L0X_ERROR_NONE) {
            ESP_LOGE(TAG, "[VHV] Failed to read interrupt status at iteration %d: %d", timeout, status);
            return status;
        }
        
        // Read range status as well for more info
        VL53L0X_ReadByte(Dev, VL53L0X_REG_RESULT_RANGE_STATUS, &range_status);
        
        // Log every 20 iterations or on first/last
        if (first_log || (timeout % 20 == 0)) {
            ESP_LOGI(TAG, "[VHV] Iter %3d: INT_STATUS=0x%02X, expecting bit 6 (0x40)",
                     timeout, interrupt_status);
            first_log = false;
        }
        
        if (timeout++ > 250) {
            ESP_LOGE(TAG, "╔════════════════════════════════════════════════╗");
            ESP_LOGE(TAG, "║  VHV CALIBRATION TIMEOUT DIAGNOSTIC            ║");
            ESP_LOGE(TAG, "╠════════════════════════════════════════════════╣");
            ESP_LOGE(TAG, "║ Iterations: %d", timeout);
            ESP_LOGE(TAG, "║ Final INT_STATUS: 0x%02X (expected bit 6: 0x40)", interrupt_status);
            ESP_LOGE(TAG, "║ Final RANGE_STATUS: 0x%02X", range_status);
            ESP_LOGE(TAG, "╠════════════════════════════════════════════════╣");
            ESP_LOGE(TAG, "║ DIAGNOSIS:                                     ║");
            
            if ((interrupt_status & 0x40) == 0) {
                ESP_LOGE(TAG, "║ • Sensor NOT completing calibration cycle     ║");
                ESP_LOGE(TAG, "║ • Possible causes:                            ║");
                ESP_LOGE(TAG, "║   1. Sensor field of view is BLOCKED          ║");
                ESP_LOGE(TAG, "║   2. Protective film still on sensor          ║");
                ESP_LOGE(TAG, "║   3. Target too close (< 5cm) or too far      ║");
                ESP_LOGE(TAG, "║   4. Insufficient light for calibration       ║");
                ESP_LOGE(TAG, "║ • SOLUTION:                                   ║");
                ESP_LOGE(TAG, "║   Point sensor at white/gray surface 20-50cm  ║");
                ESP_LOGE(TAG, "║   away in well-lit environment                ║");
            } else {
                ESP_LOGE(TAG, "║ • Unexpected: bit 6 IS set (0x%02X)", interrupt_status);
                ESP_LOGE(TAG, "║ • Should not timeout if bit is set!           ║");
            }
            
            ESP_LOGE(TAG, "╚════════════════════════════════════════════════╝");
            return VL53L0X_ERROR_TIME_OUT;
        }
        
        VL53L0X_PollingDelay(Dev, 1);
        
    } while ((interrupt_status & 0x40) == 0);  // VHV generates bit 6 on ESP32!
    
    ESP_LOGI(TAG, "[VHV] ✓ Calibration complete after %d iterations", timeout);
    ESP_LOGI(TAG, "[VHV] Final INT_STATUS: 0x%02X", interrupt_status);
    
    // Step 6: Read VHV settings
    status = VL53L0X_ReadByte(Dev, 0xCB, VhvSettings);
    if (status != VL53L0X_ERROR_NONE) {
        ESP_LOGE(TAG, "[VHV] Failed to read VHV settings: %d", status);
        return status;
    }
    ESP_LOGI(TAG, "[VHV] ✓ VHV Settings read: 0x%02X", *VhvSettings);
    
    // Step 7: Clear interrupt and reset GPIO config
    ESP_LOGI(TAG, "[VHV] Clearing interrupt and resetting GPIO config...");
    status = VL53L0X_WriteByte(Dev, VL53L0X_REG_SYSTEM_INTERRUPT_CLEAR, 0x01);
    if (status != VL53L0X_ERROR_NONE) {
        ESP_LOGE(TAG, "[VHV] Failed to clear interrupt: %d", status);
        return status;
    }
    
    status = VL53L0X_WriteByte(Dev, VL53L0X_REG_SYSRANGE_START, 0x00);
    if (status != VL53L0X_ERROR_NONE) return status;
    
    status = VL53L0X_WriteByte(Dev, VL53L0X_REG_SYSTEM_INTERRUPT_CONFIG_GPIO, 0x01);
    if (status != VL53L0X_ERROR_NONE) return status;
    
    ESP_LOGI(TAG, "[VHV] ✓ Interrupt cleared and GPIO config reset");
    
    // Delay between calibrations
    VL53L0X_PollingDelay(Dev, 5);
    
    // Step 8: Phase calibration
    ESP_LOGI(TAG, "[PHASE] Starting Phase calibration...");
    
    status = VL53L0X_WriteByte(Dev, VL53L0X_REG_SYSTEM_SEQUENCE_CONFIG, 0x02);
    if (status != VL53L0X_ERROR_NONE) {
        ESP_LOGE(TAG, "[PHASE] Failed to set SEQUENCE_CONFIG: %d", status);
        return status;
    }
    
    // Phase: write 0x01 | 0x00 = 0x01 to SYSRANGE_START
    status = VL53L0X_WriteByte(Dev, VL53L0X_REG_SYSRANGE_START, 0x01 | 0x00);
    if (status != VL53L0X_ERROR_NONE) {
        ESP_LOGE(TAG, "[PHASE] Failed to start: %d", status);
        return status;
    }
    
    VL53L0X_PollingDelay(Dev, 10);
    
    timeout = 0;
    first_log = true;
    
    do {
        status = VL53L0X_ReadByte(Dev, VL53L0X_REG_RESULT_INTERRUPT_STATUS, &interrupt_status);
        if (status != VL53L0X_ERROR_NONE) {
            ESP_LOGE(TAG, "[PHASE] Failed to read interrupt: %d", status);
            return status;
        }
        
        if (first_log || (timeout % 20 == 0)) {
            ESP_LOGI(TAG, "[PHASE] Iter %3d: INT_STATUS=0x%02X (bit6=%d, bits0-2=%d)", 
                     timeout, interrupt_status,
                     (interrupt_status & 0x40) ? 1 : 0,
                     (interrupt_status & 0x07));
            first_log = false;
        }
        
        if (timeout++ > 250) {
            ESP_LOGE(TAG, "[PHASE] Calibration timeout after %d iterations", timeout);
            ESP_LOGE(TAG, "[PHASE] Final INT_STATUS: 0x%02X", interrupt_status);
            return VL53L0X_ERROR_TIME_OUT;
        }
        
        VL53L0X_PollingDelay(Dev, 1);
        
    } while ((interrupt_status & 0x40) == 0);  // Try bit 6 for Phase on ESP32 too!
    
    ESP_LOGI(TAG, "[PHASE] ✓ Calibration complete after %d iterations", timeout);
    
    status = VL53L0X_ReadByte(Dev, 0xEE, PhaseCal);
    if (status != VL53L0X_ERROR_NONE) return status;
    
    ESP_LOGI(TAG, "[PHASE] ✓ Phase Cal read: 0x%02X", *PhaseCal);
    
    status = VL53L0X_WriteByte(Dev, VL53L0X_REG_SYSTEM_INTERRUPT_CONFIG_GPIO, 0x01);
    if (status != VL53L0X_ERROR_NONE) return status;
    
    status = VL53L0X_WriteByte(Dev, VL53L0X_REG_SYSRANGE_START, 0x00);
    if (status != VL53L0X_ERROR_NONE) return status;
    
    // Restore sequence config
    status = VL53L0X_WriteByte(Dev, VL53L0X_REG_SYSTEM_SEQUENCE_CONFIG, 0xFF);
    if (status != VL53L0X_ERROR_NONE) return status;
    
    ESP_LOGI(TAG, "═══════════════════════════════════════════════");
    ESP_LOGI(TAG, "  ✓ CALIBRATION SUCCESS");
    ESP_LOGI(TAG, "  VHV Settings: 0x%02X, Phase Cal: 0x%02X", *VhvSettings, *PhaseCal);
    ESP_LOGI(TAG, "═══════════════════════════════════════════════");
    
    return VL53L0X_ERROR_NONE;
}
