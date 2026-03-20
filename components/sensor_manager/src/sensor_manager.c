#include "sensor_manager.h"
#include "esp_log.h"
#include "vl53l0x_platform.h"

static const char *TAG = "SENSOR_MANAGER";
static i2c_master_bus_handle_t bus_handle;

// Helper to tune sensor for short range (internal)
static void tune_short_range(VL53L0X_Dev_t *dev) {
    VL53L0X_WriteByte(dev, 0x50, 0x05); // VCSEL Pulse Period Pre-Range
    VL53L0X_SetHighAccuracyMode(dev);
}

// Helper for High Precision Short Range
static void tune_high_precision(VL53L0X_Dev_t *dev) {
    // 1. High Accuracy Mode (Signal Rate Limit 0.25 MCps)
    VL53L0X_SetHighAccuracyMode(dev);

    // 2. Increase Timing Budget to 200ms (High Accuracy default is 200ms usually, but ensure it)
    VL53L0X_SetMeasurementTimingBudgetMicroSeconds(dev, 200000);

    // 3. Short Range Tuning
    // Pre-Range VCSEL Period: 14 (default) - Good for accuracy
    // Final-Range VCSEL Period: 10 (default) - Good for accuracy
    // We can try reducing Pre-Range to 12 if noise is high, but 14 is standard for accuracy.
    VL53L0X_WriteByte(dev, 0x50, 0x05); // Try this short range trick again
    
    ESP_LOGI(TAG, "Sensor Tuned for High Precision (200ms Budget)");
}

// Function to initialize a single sensor
static esp_err_t init_single_sensor(SensorConfig_t *conf, i2c_master_bus_handle_t bus, SensorMode_t mode) {
    VL53L0X_Error status;
    
    // 1. Enable Sensor (XSHUT High)
    gpio_set_level(conf->xshut_pin, 1);
    vTaskDelay(pdMS_TO_TICKS(150)); // Boot time

    // 2. Probe to see if it woke up at default address 0x29
    // Using i2c_master_probe (part of new driver)
    esp_err_t probe_ret = i2c_master_probe(bus, 0x29, 50);
    if (probe_ret != ESP_OK) {
        ESP_LOGE(TAG, "Sensor at GPIO %d FAILED to wake up (Probe Failed)", conf->xshut_pin);
        return ESP_FAIL;
    }

    // 3. Add device to the bus
    i2c_device_config_t dev_cfg = {
        .dev_addr_length = I2C_ADDR_BIT_LEN_7,
        .device_address = 0x29, // Initially 0x29
        .scl_speed_hz = 400000,
    };
    
    i2c_master_dev_handle_t dev_handle;
    ESP_ERROR_CHECK(i2c_master_bus_add_device(bus, &dev_cfg, &dev_handle));

    // Store handle in VL53L0X struct so platform layer can use it
    conf->device->DeviceSpecificParameters.I2cHandle = dev_handle;
    conf->device->DeviceSpecificParameters.I2cDevAddr = 0x29;

    // 4. Data Init
    status = VL53L0X_DataInit(conf->device);
    if (status != VL53L0X_ERROR_NONE) {
        ESP_LOGE(TAG, "DataInit failed for GPIO %d", conf->xshut_pin);
        i2c_master_bus_rm_device(dev_handle);
        return ESP_FAIL;
    }

    // 5. Static Init
    status = VL53L0X_StaticInit(conf->device);
    if (status != VL53L0X_ERROR_NONE) return ESP_FAIL;

    // 6. Perform Calibration
    uint8_t vhvSettings, phaseCal;
    VL53L0X_PerformRefCalibration(conf->device, &vhvSettings, &phaseCal);
    uint32_t refSpadCount;
    uint8_t isApertureSpads;
    VL53L0X_PerformRefSpadManagement(conf->device, &refSpadCount, &isApertureSpads);

    // 7. Change Address if needed
    if (conf->i2c_address != 0x29) {
        status = VL53L0X_SetDeviceAddress(conf->device, conf->i2c_address);
        if (status != VL53L0X_ERROR_NONE) return ESP_FAIL;
        
        // Update the I2C Device Handle with the new address!
        i2c_master_bus_rm_device(dev_handle);
        
        dev_cfg.device_address = conf->i2c_address;
        ESP_ERROR_CHECK(i2c_master_bus_add_device(bus, &dev_cfg, &dev_handle));
        
        // Update handle in struct
        conf->device->DeviceSpecificParameters.I2cHandle = dev_handle;
        conf->device->DeviceSpecificParameters.I2cDevAddr = conf->i2c_address;
        
        ESP_LOGI(TAG, "Sensor at GPIO %d moved to 0x%02X", conf->xshut_pin, conf->i2c_address);
    }

    // 8. Tuning based on Mode
    if (mode == SENSOR_MODE_HIGH_PRECISION) {
        tune_high_precision(conf->device);
    } else {
        tune_short_range(conf->device);
    }
    
    return ESP_OK;
}

esp_err_t sensor_manager_init(SensorConfig_t *configs, int count, int sda_pin, int scl_pin, uint32_t speed_hz, SensorMode_t mode) {
    // 1. Initialize I2C Master Bus
    i2c_master_bus_config_t i2c_mst_config = {
        .clk_source = I2C_CLK_SRC_DEFAULT,
        .i2c_port = -1, // Auto select
        .scl_io_num = scl_pin,
        .sda_io_num = sda_pin,
        .glitch_ignore_cnt = 7,
        .flags.enable_internal_pullup = true,
    };

    ESP_ERROR_CHECK(i2c_new_master_bus(&i2c_mst_config, &bus_handle));
    ESP_LOGI(TAG, "I2C Master Bus Initialized");

    // 2. Initialize GPIOs for XSHUT
    uint64_t pin_mask = 0;
    for (int i = 0; i < count; i++) {
        pin_mask |= (1ULL << configs[i].xshut_pin);
    }
    
    gpio_config_t io_conf = {
        .pin_bit_mask = pin_mask,
        .mode = GPIO_MODE_OUTPUT,
        .pull_up_en = GPIO_PULLUP_DISABLE,
        .pull_down_en = GPIO_PULLDOWN_DISABLE,
        .intr_type = GPIO_INTR_DISABLE
    };
    gpio_config(&io_conf);

    // 3. Reset all sensors
    for (int i = 0; i < count; i++) {
        gpio_set_level(configs[i].xshut_pin, 0);
    }
    vTaskDelay(pdMS_TO_TICKS(100));

    // 4. Initialize Each Sensor Sequentially
    for (int i = 0; i < count; i++) {
        ESP_LOGI(TAG, "Initializing Sensor %d (GPIO %d)...", i+1, configs[i].xshut_pin);
        if (init_single_sensor(&configs[i], bus_handle, mode) == ESP_OK) {
            configs[i].active = true;
            ESP_LOGI(TAG, "Sensor %d Init Success!", i+1);
        } else {
            configs[i].active = false;
            ESP_LOGE(TAG, "Sensor %d Init Failed - Skipping", i+1);
        }
        vTaskDelay(pdMS_TO_TICKS(100));
    }
    
    return ESP_OK;
}

void sensor_manager_start_continuous(SensorConfig_t *configs, int count) {
    for (int i = 0; i < count; i++) {
        if (configs[i].active) {
            VL53L0X_SetDeviceMode(configs[i].device, VL53L0X_DEVICEMODE_CONTINUOUS_RANGING);
            VL53L0X_StartMeasurement(configs[i].device);
            vTaskDelay(pdMS_TO_TICKS(50)); // Stagger start
        }
    }
}

void sensor_manager_read_all(SensorConfig_t *configs, int count, VL53L0X_RangingMeasurementData_t *data) {
    for (int i = 0; i < count; i++) {
        if (configs[i].active) {
            VL53L0X_GetRangingMeasurementData(configs[i].device, &data[i]);
            VL53L0X_ClearInterruptMask(configs[i].device, 0);
            
            // Apply Manual Offset Correction
            // Apply even if status is not 0 (e.g. Phase Fail might still have valid-ish range)
            // But be careful with error codes like 8190
            if (data[i].RangeMilliMeter < 8000) { 
                int32_t corrected_range = (int32_t)data[i].RangeMilliMeter - configs[i].offset_mm;
                if (corrected_range < 0) corrected_range = 0;
                data[i].RangeMilliMeter = (uint16_t)corrected_range;
            }
        } else {
            data[i].RangeMilliMeter = -1; // Indicate failure
            data[i].RangeStatus = 255;
        }
    }
}
