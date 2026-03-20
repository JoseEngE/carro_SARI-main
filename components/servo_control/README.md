# Servo Control Component

This component provides a simple interface to control PWM servomotors (like the MG90S) using the ESP32's LEDC (LED Control) peripheral.

## Features

*   **PWM Control**: Uses hardware LEDC for stable PWM signals.
*   **Angle Abstraction**: Allows setting the servo position directly in degrees (0-180).
*   **Configurable**: Supports custom GPIO, timer, channel, frequency, and pulse width ranges.

## API Reference

### Data Structures

#### `servo_config_t`

Configuration structure for initializing the servo.

| Field | Type | Description |
| :--- | :--- | :--- |
| `gpio_num` | `int` | GPIO pin number connected to the servo signal wire. |
| `timer_number` | `ledc_timer_t` | LEDC timer source (e.g., `LEDC_TIMER_0`). |
| `channel_number` | `ledc_channel_t` | LEDC channel (e.g., `LEDC_CHANNEL_0`). |
| `min_pulse_width_us` | `int` | Pulse width in microseconds for 0 degrees (e.g., 500). |
| `max_pulse_width_us` | `int` | Pulse width in microseconds for 180 degrees (e.g., 2400). |
| `frequency` | `int` | PWM frequency in Hz (typically 50Hz for servos). |

### Functions

#### `esp_err_t servo_init(const servo_config_t *config)`

Initializes the servo control component with the provided configuration.

*   **Parameters**:
    *   `config`: Pointer to a `servo_config_t` structure.
*   **Returns**:
    *   `ESP_OK` on success.
    *   `ESP_ERR_INVALID_ARG` if config is NULL.
    *   `ESP_ERR_NO_MEM` if memory allocation fails.
    *   Other `ESP_ERR_*` codes from LEDC driver.

#### `esp_err_t servo_set_angle(float angle)`

Sets the servo motor to the specified angle.

*   **Parameters**:
    *   `angle`: Target angle in degrees (0.0 to 180.0).
*   **Returns**:
    *   `ESP_OK` on success.
    *   `ESP_ERR_INVALID_STATE` if not initialized.

## Example Usage

```c
#include "servo_control.h"

void app_main(void)
{
    // Configure the servo
    servo_config_t servo_cfg = {
        .gpio_num = 18,
        .timer_number = LEDC_TIMER_0,
        .channel_number = LEDC_CHANNEL_0,
        .min_pulse_width_us = 500,  // MG90S specific
        .max_pulse_width_us = 2400, // MG90S specific
        .frequency = 50
    };

    // Initialize
    ESP_ERROR_CHECK(servo_init(&servo_cfg));

    // Move to 90 degrees
    servo_set_angle(90.0f);
}
```
