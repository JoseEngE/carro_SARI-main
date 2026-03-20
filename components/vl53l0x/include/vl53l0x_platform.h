/*
 * VL53L0X Platform Abstraction Layer for ESP32
 * I2C and timing functions
 */

#ifndef VL53L0X_PLATFORM_H_
#define VL53L0X_PLATFORM_H_

#include "vl53l0x_types.h"
#include "driver/i2c.h"

#ifdef __cplusplus
extern "C" {
#endif

/**
 * @brief Initialize I2C bus for VL53L0X communication
 * 
 * @param i2c_port I2C port number
 * @param sda_pin SDA pin number
 * @param scl_pin SCL pin number
 * @param speed_hz I2C speed in Hz
 * @return VL53L0X_ERROR_NONE on success
 */
VL53L0X_Error VL53L0X_PlatformInit(i2c_port_t i2c_port, int sda_pin, int scl_pin, uint32_t speed_hz);

/**
 * @brief Deinitialize I2C bus
 * 
 * @param i2c_port I2C port number
 * @return VL53L0X_ERROR_NONE on success
 */
VL53L0X_Error VL53L0X_PlatformDeInit(i2c_port_t i2c_port);

/**
 * @brief Write multiple bytes to device via I2C
 * 
 * @param Dev Device handle
 * @param index Register address
 * @param pdata Pointer to data buffer
 * @param count Number of bytes to write
 * @return VL53L0X_ERROR_NONE on success
 */
VL53L0X_Error VL53L0X_WriteMulti(VL53L0X_DEV Dev, uint8_t index, uint8_t *pdata, uint32_t count);

/**
 * @brief Read multiple bytes from device via I2C
 * 
 * @param Dev Device handle
 * @param index Register address
 * @param pdata Pointer to data buffer
 * @param count Number of bytes to read
 * @return VL53L0X_ERROR_NONE on success
 */
VL53L0X_Error VL53L0X_ReadMulti(VL53L0X_DEV Dev, uint8_t index, uint8_t *pdata, uint32_t count);

/**
 * @brief Write single byte to device
 * 
 * @param Dev Device handle
 * @param index Register address
 * @param data Data byte
 * @return VL53L0X_ERROR_NONE on success
 */
VL53L0X_Error VL53L0X_WriteByte(VL53L0X_DEV Dev, uint8_t index, uint8_t data);

/**
 * @brief Write word (16-bit) to device
 * 
 * @param Dev Device handle
 * @param index Register address
 * @param data Data word
 * @return VL53L0X_ERROR_NONE on success
 */
VL53L0X_Error VL53L0X_WriteWord(VL53L0X_DEV Dev, uint8_t index, uint16_t data);

/**
 * @brief Write double word (32-bit) to device
 * 
 * @param Dev Device handle
 * @param index Register address
 * @param data Data double word
 * @return VL53L0X_ERROR_NONE on success
 */
VL53L0X_Error VL53L0X_WriteDWord(VL53L0X_DEV Dev, uint8_t index, uint32_t data);

/**
 * @brief Read single byte from device
 * 
 * @param Dev Device handle
 * @param index Register address
 * @param pdata Pointer to data byte
 * @return VL53L0X_ERROR_NONE on success
 */
VL53L0X_Error VL53L0X_ReadByte(VL53L0X_DEV Dev, uint8_t index, uint8_t *pdata);

/**
 * @brief Read word (16-bit) from device
 * 
 * @param Dev Device handle
 * @param index Register address
 * @param pdata Pointer to data word
 * @return VL53L0X_ERROR_NONE on success
 */
VL53L0X_Error VL53L0X_ReadWord(VL53L0X_DEV Dev, uint8_t index, uint16_t *pdata);

/**
 * @brief Read double word (32-bit) from device
 * 
 * @param Dev Device handle
 * @param index Register address
 * @param pdata Pointer to data double word
 * @return VL53L0X_ERROR_NONE on success
 */
VL53L0X_Error VL53L0X_ReadDWord(VL53L0X_DEV Dev, uint8_t index, uint32_t *pdata);

/**
 * @brief Polling delay in milliseconds
 * 
 * @param Dev Device handle
 * @param ms Delay in milliseconds
 * @return VL53L0X_ERROR_NONE on success
 */
VL53L0X_Error VL53L0X_PollingDelay(VL53L0X_DEV Dev, uint32_t ms);

/**
 * @brief Get current tick count in milliseconds
 * 
 * @param ptick_count_ms Pointer to tick count
 * @return VL53L0X_ERROR_NONE on success
 */
VL53L0X_Error VL53L0X_GetTickCount(uint32_t *ptick_count_ms);

#ifdef __cplusplus
}
#endif

#endif /* VL53L0X_PLATFORM_H_ */
