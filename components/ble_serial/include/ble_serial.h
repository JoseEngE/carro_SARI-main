#ifndef BLE_SERIAL_H
#define BLE_SERIAL_H

#include <stdint.h>
#include <stdbool.h>

#ifdef __cplusplus
extern "C" {
#endif

// Initializes the NimBLE stack and starts advertising as "Sensores-BLE"
void ble_serial_init(void);

// Formats and sends a string to the connected Bluetooth client via notification
void ble_serial_print(const char *format, ...);

// Returns true if a phone/app is currently connected
bool ble_serial_is_connected(void);

#ifdef __cplusplus
}
#endif

#endif // BLE_SERIAL_H
