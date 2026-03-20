#ifndef WEB_CONTROL_H
#define WEB_CONTROL_H

#include "esp_err.h"
#include "esp_http_server.h"

#ifdef __cplusplus
extern "C" {
#endif

/**
 * @brief Web control configuration structure
 */
typedef struct {
    const char *wifi_ssid;          /*!< WiFi AP SSID */
    const char *wifi_password;      /*!< WiFi AP password */
    uint8_t wifi_channel;           /*!< WiFi channel (1-13) */
    uint8_t max_connections;        /*!< Maximum simultaneous connections */
    uint16_t server_port;           /*!< HTTP server port (default: 80) */
    uint32_t motor_timeout_ms;      /*!< Motor stop timeout in ms (default: 500) */
} web_control_config_t;

/**
 * @brief Motor control callback function type
 * 
 * @param throttle Throttle value (-100 to +100)
 * @param steering Steering value (-100 to +100)
 */
typedef void (*web_control_motor_callback_t)(int8_t throttle, int8_t steering);

/**
 * @brief Mode control callback function type
 * 
 * @param mode Mode value (0 = manual, 1 = auto)
 */
typedef void (*web_control_mode_callback_t)(uint8_t mode);

/**
 * @brief Speed limit control callback function type
 * 
 * @param speed_limit Speed limit percentage (0-100)
 */
typedef void (*web_control_speed_limit_callback_t)(uint8_t speed_limit);

/**
 * @brief Default configuration for web control
 */
#define WEB_CONTROL_DEFAULT_CONFIG() {      \
    .wifi_ssid = "RC_Car_Control",          \
    .wifi_password = "rccar123",            \
    .wifi_channel = 1,                      \
    .max_connections = 4,                   \
    .server_port = 80,                      \
    .motor_timeout_ms = 500,                \
}

/**
 * @brief Initialize web control component
 * 
 * @param config Pointer to configuration structure
 * @return esp_err_t ESP_OK on success
 */
esp_err_t web_control_init(const web_control_config_t *config);

/**
 * @brief Start the web server
 * 
 * @return esp_err_t ESP_OK on success
 */
esp_err_t web_control_start(void);

/**
 * @brief Stop the web server
 * 
 * @return esp_err_t ESP_OK on success
 */
esp_err_t web_control_stop(void);

/**
 * @brief Set motor control callback
 * 
 * @param callback Function to call when motor commands are received
 * @return esp_err_t ESP_OK on success
 */
esp_err_t web_control_set_motor_callback(web_control_motor_callback_t callback);

/**
 * @brief Set mode control callback
 * 
 * @param callback Function to call when mode toggle commands are received
 * @return esp_err_t ESP_OK on success
 */
esp_err_t web_control_set_mode_callback(web_control_mode_callback_t callback);

/**
 * @brief Set speed limit control callback
 * 
 * @param callback Function to call when speed limit updates are received
 * @return esp_err_t ESP_OK on success
 */
esp_err_t web_control_set_speed_limit_callback(web_control_speed_limit_callback_t callback);

/**
 * @brief Get current connection status
 * 
 * @return true if client is connected, false otherwise
 */
bool web_control_is_connected(void);

/**
 * @brief Send telemetry data to connected clients
 * 
 * @param battery_percent Battery percentage (0-100)
 * @param speed_kmh Current speed in km/h
 * @param signal_strength Signal strength (0-100)
 * @param dist_left Distance left (mm)
 * @param dist_center Distance center (mm)
 * @param dist_right Distance right (mm)
 * @return esp_err_t ESP_OK on success
 */
esp_err_t web_control_send_telemetry(uint8_t battery_percent, float speed_kmh, uint8_t signal_strength, uint16_t dist_left, uint16_t dist_center, uint16_t dist_right);

#ifdef __cplusplus
}
#endif

#endif // WEB_CONTROL_H
