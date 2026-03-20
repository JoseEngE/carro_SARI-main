/*
 * VL53L0X Core API Implementation for ESP32
 * Adapted from STMicroelectronics VL53L0X API
 */

#include "vl53l0x.h"
#include "driver/gpio.h"
#include "esp_log.h"
#include <string.h>

static const char *TAG = "VL53L0X_API";
