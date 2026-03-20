/*
 * VL53L0X Types Definition for ESP32
 * Platform-specific types for VL53L0X ToF sensor
 */

#ifndef VL53L0X_TYPES_H_
#define VL53L0X_TYPES_H_

#include <stdint.h>
#include <stddef.h>
#include <stdbool.h>

#ifdef __cplusplus
extern "C" {
#endif

/** @defgroup VL53L0X_define_Error_group Error and Warning code returned by API
 *  The following DEFINE are used to identify the PAL ERROR
 *  @{
 */
typedef int8_t VL53L0X_Error;

#define VL53L0X_ERROR_NONE                          ((VL53L0X_Error)  0)
#define VL53L0X_ERROR_CALIBRATION_WARNING           ((VL53L0X_Error) -1)
#define VL53L0X_ERROR_MIN_CLIPPED                   ((VL53L0X_Error) -2)
#define VL53L0X_ERROR_UNDEFINED                     ((VL53L0X_Error) -3)
#define VL53L0X_ERROR_INVALID_PARAMS                ((VL53L0X_Error) -4)
#define VL53L0X_ERROR_NOT_SUPPORTED                 ((VL53L0X_Error) -5)
#define VL53L0X_ERROR_RANGE_ERROR                   ((VL53L0X_Error) -6)
#define VL53L0X_ERROR_TIME_OUT                      ((VL53L0X_Error) -7)
#define VL53L0X_ERROR_MODE_NOT_SUPPORTED            ((VL53L0X_Error) -8)
#define VL53L0X_ERROR_BUFFER_TOO_SMALL              ((VL53L0X_Error) -9)
#define VL53L0X_ERROR_GPIO_NOT_EXISTING             ((VL53L0X_Error) -10)
#define VL53L0X_ERROR_GPIO_FUNCTIONALITY_NOT_SUPPORTED  ((VL53L0X_Error) -11)
#define VL53L0X_ERROR_INTERRUPT_NOT_CLEARED         ((VL53L0X_Error) -12)
#define VL53L0X_ERROR_CONTROL_INTERFACE             ((VL53L0X_Error) -20)
#define VL53L0X_ERROR_INVALID_COMMAND               ((VL53L0X_Error) -30)
#define VL53L0X_ERROR_DIVISION_BY_ZERO              ((VL53L0X_Error) -40)
#define VL53L0X_ERROR_REF_SPAD_INIT                 ((VL53L0X_Error) -50)
#define VL53L0X_ERROR_NOT_IMPLEMENTED               ((VL53L0X_Error) -99)

/** @} VL53L0X_define_Error_group */

/** @defgroup VL53L0X_define_DeviceModes_group Defines Device modes
 *  Defines all possible modes for the device
 *  @{
 */
typedef uint8_t VL53L0X_DeviceModes;

#define VL53L0X_DEVICEMODE_SINGLE_RANGING           ((VL53L0X_DeviceModes)  0)
#define VL53L0X_DEVICEMODE_CONTINUOUS_RANGING       ((VL53L0X_DeviceModes)  1)
#define VL53L0X_DEVICEMODE_SINGLE_HISTOGRAM         ((VL53L0X_DeviceModes)  2)
#define VL53L0X_DEVICEMODE_CONTINUOUS_TIMED_RANGING ((VL53L0X_DeviceModes)  3)
#define VL53L0X_DEVICEMODE_SINGLE_ALS               ((VL53L0X_DeviceModes) 10)
#define VL53L0X_DEVICEMODE_GPIO_DRIVE               ((VL53L0X_DeviceModes) 20)
#define VL53L0X_DEVICEMODE_GPIO_OSC                 ((VL53L0X_DeviceModes) 21)

/** @} VL53L0X_define_DeviceModes_group */

/** @defgroup VL53L0X_define_RangeStatus_group Defines the Range Status
 *  @{
 */
#define VL53L0X_RANGESTATUS_RANGE_VALID             0
#define VL53L0X_RANGESTATUS_SIGMA_FAIL              1
#define VL53L0X_RANGESTATUS_SIGNAL_FAIL             2
#define VL53L0X_RANGESTATUS_MIN_RANGE_FAIL          3
#define VL53L0X_RANGESTATUS_PHASE_FAIL              4
#define VL53L0X_RANGESTATUS_HW_FAIL                 5

/** @} VL53L0X_define_RangeStatus_group */

/**
 * @brief VL53L0X Device Information
 */
typedef struct {
    char Name[32];
    char Type[32];
    char ProductId[32];
    uint8_t ProductType;
    uint8_t ProductRevisionMajor;
    uint8_t ProductRevisionMinor;
} VL53L0X_DeviceInfo_t;

/**
 * @brief VL53L0X Device Parameters
 */
typedef struct {
    VL53L0X_DeviceModes DeviceMode;
    uint8_t HistogramMode;
    uint32_t MeasurementTimingBudgetMicroSeconds;
    uint32_t InterMeasurementPeriodMilliSeconds;
    uint8_t XTalkCompensationEnable;
    int32_t XTalkCompensationRateMegaCps;
    int32_t RangeOffsetMicroMeters;
    uint16_t LimitChecksEnable;
    uint8_t LimitChecksStatus;
} VL53L0X_DeviceParameters_t;

/**
 * @brief VL53L0X Ranging Measurement Data
 */
typedef struct {
    uint32_t TimeStamp;
    uint32_t MeasurementTimeUsec;
    uint16_t RangeMilliMeter;
    uint16_t RangeDMaxMilliMeter;
    uint32_t SignalRateRtnMegaCps;
    uint32_t AmbientRateRtnMegaCps;
    uint16_t EffectiveSpadRtnCount;
    uint8_t ZoneId;
    uint8_t RangeFractionalPart;
    uint8_t RangeStatus;
} VL53L0X_RangingMeasurementData_t;

/**
 * @brief VL53L0X Device Specific Parameters
 */
typedef struct {
    uint8_t I2cDevAddr;
    uint8_t CommsType;
    uint16_t CommsSpeedKhz;
    void *I2cHandle;
} VL53L0X_DeviceSpecificParameters_t;

/**
 * @brief VL53L0X Device Structure
 */
typedef struct {
    VL53L0X_DeviceParameters_t CurrentParameters;
    VL53L0X_DeviceSpecificParameters_t DeviceSpecificParameters;
    uint8_t Data[256];  // Internal data buffer
    uint8_t PalState;
} VL53L0X_Dev_t;

typedef VL53L0X_Dev_t *VL53L0X_DEV;

#ifdef __cplusplus
}
#endif

#endif /* VL53L0X_TYPES_H_ */
