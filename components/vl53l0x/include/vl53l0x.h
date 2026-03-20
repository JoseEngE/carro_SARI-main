/*
 * VL53L0X API for ESP32
 * Main API header file
 */

#ifndef VL53L0X_H_
#define VL53L0X_H_

#include "vl53l0x_types.h"
#include "vl53l0x_platform.h"

#ifdef __cplusplus
extern "C" {
#endif

/** Default I2C address */
#define VL53L0X_DEFAULT_ADDRESS     0x29

/**
 * @brief Data Initialization
 * 
 * @param Dev Device handle
 * @return VL53L0X_ERROR_NONE on success
 */
VL53L0X_Error VL53L0X_DataInit(VL53L0X_DEV Dev);

/**
 * @brief Static Initialization
 * 
 * @param Dev Device handle
 * @return VL53L0X_ERROR_NONE on success
 */
VL53L0X_Error VL53L0X_StaticInit(VL53L0X_DEV Dev);

/**
 * @brief Get Device Info
 * 
 * @param Dev Device handle
 * @param pVL53L0X_DeviceInfo Pointer to device info structure
 * @return VL53L0X_ERROR_NONE on success
 */
VL53L0X_Error VL53L0X_GetDeviceInfo(VL53L0X_DEV Dev, VL53L0X_DeviceInfo_t *pVL53L0X_DeviceInfo);

/**
 * @brief Perform Reference Calibration
 * 
 * @param Dev Device handle
 * @param VhvSettings Pointer to VHV settings
 * @param PhaseCal Pointer to phase calibration
 * @return VL53L0X_ERROR_NONE on success
 */
VL53L0X_Error VL53L0X_PerformRefCalibration(VL53L0X_DEV Dev, uint8_t *VhvSettings, uint8_t *PhaseCal);

/**
 * @brief Perform Reference Calibration with Detailed Debug Logging
 * Alternative version with extensive diagnostics for troubleshooting calibration issues
 * 
 * @param Dev Device handle
 * @param VhvSettings Pointer to VHV settings
 * @param PhaseCal Pointer to phase calibration
 * @return VL53L0X_ERROR_NONE on success
 */
VL53L0X_Error VL53L0X_PerformRefCalibration_Debug(VL53L0X_DEV Dev, uint8_t *VhvSettings, uint8_t *PhaseCal);

/**
 * @brief Perform Reference SPAD Management
 * 
 * @param Dev Device handle
 * @param refSpadCount Pointer to reference SPAD count
 * @param isApertureSpads Pointer to aperture SPADs flag
 * @return VL53L0X_ERROR_NONE on success
 */
VL53L0X_Error VL53L0X_PerformRefSpadManagement(VL53L0X_DEV Dev, uint32_t *refSpadCount, uint8_t *isApertureSpads);

/**
 * @brief Set Device Mode
 * 
 * @param Dev Device handle
 * @param DeviceMode Device mode
 * @return VL53L0X_ERROR_NONE on success
 */
VL53L0X_Error VL53L0X_SetDeviceMode(VL53L0X_DEV Dev, VL53L0X_DeviceModes DeviceMode);

/**
 * @brief Get Device Mode
 * 
 * @param Dev Device handle
 * @param pDeviceMode Pointer to device mode
 * @return VL53L0X_ERROR_NONE on success
 */
VL53L0X_Error VL53L0X_GetDeviceMode(VL53L0X_DEV Dev, VL53L0X_DeviceModes *pDeviceMode);

/**
 * @brief Start Measurement
 * 
 * @param Dev Device handle
 * @return VL53L0X_ERROR_NONE on success
 */
VL53L0X_Error VL53L0X_StartMeasurement(VL53L0X_DEV Dev);

/**
 * @brief Stop Measurement
 * 
 * @param Dev Device handle
 * @return VL53L0X_ERROR_NONE on success
 */
VL53L0X_Error VL53L0X_StopMeasurement(VL53L0X_DEV Dev);

/**
 * @brief Get Ranging Measurement Data
 * 
 * @param Dev Device handle
 * @param pRangingMeasurementData Pointer to ranging measurement data
 * @return VL53L0X_ERROR_NONE on success
 */
VL53L0X_Error VL53L0X_GetRangingMeasurementData(VL53L0X_DEV Dev, VL53L0X_RangingMeasurementData_t *pRangingMeasurementData);

/**
 * @brief Perform Single Ranging Measurement
 * 
 * @param Dev Device handle
 * @param pRangingMeasurementData Pointer to ranging measurement data
 * @return VL53L0X_ERROR_NONE on success
 */
VL53L0X_Error VL53L0X_PerformSingleRangingMeasurement(VL53L0X_DEV Dev, VL53L0X_RangingMeasurementData_t *pRangingMeasurementData);

/**
 * @brief Clear Interrupt Mask
 * 
 * @param Dev Device handle
 * @param InterruptMask Interrupt mask
 * @return VL53L0X_ERROR_NONE on success
 */
VL53L0X_Error VL53L0X_ClearInterruptMask(VL53L0X_DEV Dev, uint32_t InterruptMask);

/**
 * @brief Get Interrupt Mask Status
 * 
 * @param Dev Device handle
 * @param pInterruptMaskStatus Pointer to interrupt mask status
 * @return VL53L0X_ERROR_NONE on success
 */
VL53L0X_Error VL53L0X_GetInterruptMaskStatus(VL53L0X_DEV Dev, uint32_t *pInterruptMaskStatus);

/**
 * @brief Set Measurement Timing Budget Microseconds
 * 
 * @param Dev Device handle
 * @param MeasurementTimingBudgetMicroSeconds Timing budget in microseconds
 * @return VL53L0X_ERROR_NONE on success
 */
VL53L0X_Error VL53L0X_SetMeasurementTimingBudgetMicroSeconds(VL53L0X_DEV Dev, uint32_t MeasurementTimingBudgetMicroSeconds);

/**
 * @brief Get Measurement Timing Budget Microseconds
 * 
 * @param Dev Device handle
 * @param pMeasurementTimingBudgetMicroSeconds Pointer to timing budget
 * @return VL53L0X_ERROR_NONE on success
 */
VL53L0X_Error VL53L0X_GetMeasurementTimingBudgetMicroSeconds(VL53L0X_DEV Dev, uint32_t *pMeasurementTimingBudgetMicroSeconds);

/**
 * @brief Set Inter Measurement Period Milliseconds
 * 
 * @param Dev Device handle
 * @param InterMeasurementPeriodMilliSeconds Inter measurement period in milliseconds
 * @return VL53L0X_ERROR_NONE on success
 */
VL53L0X_Error VL53L0X_SetInterMeasurementPeriodMilliSeconds(VL53L0X_DEV Dev, uint32_t InterMeasurementPeriodMilliSeconds);

/**
 * @brief Get Inter Measurement Period Milliseconds
 * 
 * @param Dev Device handle
 * @param pInterMeasurementPeriodMilliSeconds Pointer to inter measurement period
 * @return VL53L0X_ERROR_NONE on success
 */
VL53L0X_Error VL53L0X_GetInterMeasurementPeriodMilliSeconds(VL53L0X_DEV Dev, uint32_t *pInterMeasurementPeriodMilliSeconds);

/**
 * @brief Set Limit Check Enable
 * 
 * @param Dev Device handle
 * @param LimitCheckId Limit check ID
 * @param LimitCheckEnable Limit check enable flag
 * @return VL53L0X_ERROR_NONE on success
 */
VL53L0X_Error VL53L0X_SetLimitCheckEnable(VL53L0X_DEV Dev, uint16_t LimitCheckId, uint8_t LimitCheckEnable);

/**
 * @brief Set Limit Check Value
 * 
 * @param Dev Device handle
 * @param LimitCheckId Limit check ID
 * @param LimitCheckValue Limit check value
 * @return VL53L0X_ERROR_NONE on success
 */
VL53L0X_Error VL53L0X_SetLimitCheckValue(VL53L0X_DEV Dev, uint16_t LimitCheckId, uint32_t LimitCheckValue);

/**
 * @brief High Accuracy Mode Configuration
 * Higher precision, slower measurements
 * 
 * @param Dev Device handle
 * @return VL53L0X_ERROR_NONE on success
 */
VL53L0X_Error VL53L0X_SetHighAccuracyMode(VL53L0X_DEV Dev);

/**
 * @brief Long Range Mode Configuration
 * Maximum range ~2m, slower measurements
 * 
 * @param Dev Device handle
 * @return VL53L0X_ERROR_NONE on success
 */
VL53L0X_Error VL53L0X_SetLongRangeMode(VL53L0X_DEV Dev);

/**
 * @brief High Speed Mode Configuration
 * Faster measurements, lower accuracy
 * 
 * @param Dev Device handle
 * @return VL53L0X_ERROR_NONE on success
 */
VL53L0X_Error VL53L0X_SetHighSpeedMode(VL53L0X_DEV Dev);

/**
 * @brief Default Mode Configuration
 * Balanced speed and accuracy
 * 
 * @param Dev Device handle
 * @return VL53L0X_ERROR_NONE on success
 */
VL53L0X_Error VL53L0X_SetDefaultMode(VL53L0X_DEV Dev);

/**
 * @brief Set new I2C device address
 * 
 * @param Dev Device handle
 * @param new_address New I2C address (must be different from default 0x29)
 * @return VL53L0X_ERROR_NONE on success
 */
VL53L0X_Error VL53L0X_SetDeviceAddress(VL53L0X_DEV Dev, uint8_t new_address);

/**
 * @brief Initialize GPIO pin for XSHUT control
 * 
 * @param xshut_pin GPIO pin number for XSHUT
 * @return VL53L0X_ERROR_NONE on success
 */
VL53L0X_Error VL53L0X_InitXShutPin(int xshut_pin);

/**
 * @brief Set XSHUT pin state (enable/disable sensor)
 * 
 * @param xshut_pin GPIO pin number
 * @param enable true = sensor enabled (HIGH), false = sensor disabled (LOW)
 * @return VL53L0X_ERROR_NONE on success
 */
VL53L0X_Error VL53L0X_SetXShutState(int xshut_pin, bool enable);

#ifdef __cplusplus
}
#endif

#endif /* VL53L0X_H_ */
