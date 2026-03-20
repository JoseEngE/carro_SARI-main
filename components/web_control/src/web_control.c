#include "web_control.h"
#include "esp_wifi.h"
#include "esp_event.h"
#include "esp_log.h"
#include "esp_system.h"
#include "nvs_flash.h"
#include "esp_netif.h"
#include "lwip/err.h"
#include "lwip/sys.h"
#include "esp_timer.h"
#include "esp_mac.h"
#include <string.h>

static const char *TAG = "web_control";

// External declarations for embedded files
extern const uint8_t index_html_start[] asm("_binary_index_html_start");
extern const uint8_t index_html_end[] asm("_binary_index_html_end");
extern const uint8_t style_css_start[] asm("_binary_style_css_start");
extern const uint8_t style_css_end[] asm("_binary_style_css_end");
extern const uint8_t app_js_start[] asm("_binary_app_js_start");
extern const uint8_t app_js_end[] asm("_binary_app_js_end");

// Global variables
static httpd_handle_t server = NULL;
static web_control_config_t g_config;
static web_control_motor_callback_t g_motor_callback = NULL;
static web_control_mode_callback_t g_mode_callback = NULL;
static web_control_speed_limit_callback_t g_speed_limit_callback = NULL;
static bool g_client_connected = false;
static esp_timer_handle_t g_timeout_timer = NULL;
static int64_t g_last_command_time = 0;

// Telemetry data
static uint8_t g_battery = 87;
static float g_speed = 0.0;
static uint8_t g_signal = 100;
static uint16_t g_dist_left = 8190;
static uint16_t g_dist_center = 8190;
static uint16_t g_dist_right = 8190;

// Forward declarations
static void wifi_event_handler(void* arg, esp_event_base_t event_base, int32_t event_id, void* event_data);
static esp_err_t command_handler(httpd_req_t *req);
static esp_err_t telemetry_handler(httpd_req_t *req);
static void motor_timeout_callback(void* arg);

/**
 * @brief Initialize WiFi in AP mode
 */
static esp_err_t wifi_init_softap(void)
{
    esp_netif_create_default_wifi_ap();

    wifi_init_config_t cfg = WIFI_INIT_CONFIG_DEFAULT();
    ESP_ERROR_CHECK(esp_wifi_init(&cfg));

    ESP_ERROR_CHECK(esp_event_handler_instance_register(WIFI_EVENT,
                                                        ESP_EVENT_ANY_ID,
                                                        &wifi_event_handler,
                                                        NULL,
                                                        NULL));

    wifi_config_t wifi_config = {
        .ap = {
            .ssid_len = strlen(g_config.wifi_ssid),
            .channel = g_config.wifi_channel,
            .max_connection = g_config.max_connections,
            .authmode = WIFI_AUTH_WPA2_PSK,
            .pmf_cfg = {
                .required = false,
            },
        },
    };
    
    strcpy((char *)wifi_config.ap.ssid, g_config.wifi_ssid);
    strcpy((char *)wifi_config.ap.password, g_config.wifi_password);

    if (strlen(g_config.wifi_password) == 0) {
        wifi_config.ap.authmode = WIFI_AUTH_OPEN;
    }

    ESP_ERROR_CHECK(esp_wifi_set_mode(WIFI_MODE_AP));
    ESP_ERROR_CHECK(esp_wifi_set_config(WIFI_IF_AP, &wifi_config));
    ESP_ERROR_CHECK(esp_wifi_start());

    ESP_LOGI(TAG, "WiFi AP started. SSID:%s password:%s channel:%d",
             g_config.wifi_ssid, g_config.wifi_password, g_config.wifi_channel);

    return ESP_OK;
}

/**
 * @brief WiFi event handler
 */
static void wifi_event_handler(void* arg, esp_event_base_t event_base,
                              int32_t event_id, void* event_data)
{
    if (event_id == WIFI_EVENT_AP_STACONNECTED) {
        wifi_event_ap_staconnected_t* event = (wifi_event_ap_staconnected_t*) event_data;
        ESP_LOGI(TAG, "Station " MACSTR " joined, AID=%d",
                 MAC2STR(event->mac), event->aid);
        g_client_connected = true;
    } else if (event_id == WIFI_EVENT_AP_STADISCONNECTED) {
        wifi_event_ap_stadisconnected_t* event = (wifi_event_ap_stadisconnected_t*) event_data;
        ESP_LOGI(TAG, "Station " MACSTR " left, AID=%d",
                 MAC2STR(event->mac), event->aid);
        g_client_connected = false;
    }
}

/**
 * @brief HTTP GET handler for index.html
 */
static esp_err_t index_handler(httpd_req_t *req)
{
    httpd_resp_set_type(req, "text/html");
    httpd_resp_send(req, (const char *)index_html_start, index_html_end - index_html_start);
    return ESP_OK;
}

/**
 * @brief HTTP GET handler for style.css
 */
static esp_err_t css_handler(httpd_req_t *req)
{
    httpd_resp_set_type(req, "text/css");
    httpd_resp_send(req, (const char *)style_css_start, style_css_end - style_css_start);
    return ESP_OK;
}

/**
 * @brief HTTP GET handler for app.js
 */
static esp_err_t js_handler(httpd_req_t *req)
{
    httpd_resp_set_type(req, "application/javascript");
    httpd_resp_send(req, (const char *)app_js_start, app_js_end - app_js_start);
    return ESP_OK;
}

/**
 * @brief Motor timeout callback - stops motors if no command received
 */
static void motor_timeout_callback(void* arg)
{
    int64_t now = esp_timer_get_time() / 1000; // Convert to ms
    if (g_client_connected && (now - g_last_command_time > g_config.motor_timeout_ms)) {
        ESP_LOGW(TAG, "Motor command timeout - stopping motors");
        if (g_motor_callback) {
            g_motor_callback(0, 0); // Stop motors
        }
    }
}

/**
 * @brief HTTP POST handler for motor commands
 */
static esp_err_t command_handler(httpd_req_t *req)
{
    char buf[100];
    int ret, remaining = req->content_len;

    if (remaining >= sizeof(buf)) {
        httpd_resp_send_err(req, HTTPD_400_BAD_REQUEST, "Content too long");
        return ESP_FAIL;
    }

    ret = httpd_req_recv(req, buf, remaining);
    if (ret <= 0) {
        if (ret == HTTPD_SOCK_ERR_TIMEOUT) {
            httpd_resp_send_408(req);
        }
        return ESP_FAIL;
    }

    // Parse binary data
    if (ret >= 1) {
        uint8_t msg_type = buf[0];
        
        if (msg_type == 0x01 && ret >= 3) { // Motor control
            int8_t throttle = (int8_t)buf[1];
            int8_t steering = (int8_t)buf[2];
            
            // Validate range
            if (throttle < -100) throttle = -100;
            if (throttle > 100) throttle = 100;
            if (steering < -100) steering = -100;
            if (steering > 100) steering = 100;
            
            // Update last command time
            g_last_command_time = esp_timer_get_time() / 1000;
            
            // Call motor callback
            if (g_motor_callback) {
                g_motor_callback(throttle, steering);
            }
            
        } else if (msg_type == 0x03) { // Emergency stop
            ESP_LOGW(TAG, "Emergency stop received");
            if (g_motor_callback) {
                g_motor_callback(0, 0);
            }
        } else if (msg_type == 0x04 && ret >= 2) { // Mode toggle
            uint8_t mode = buf[1];
            ESP_LOGI(TAG, "Mode toggle received: %d", mode);
            if (g_mode_callback) {
                g_mode_callback(mode);
            }
        } else if (msg_type == 0x05 && ret >= 2) { // Speed limit
            uint8_t limit = buf[1];
            ESP_LOGI(TAG, "Speed limit received: %d", limit);
            if (g_speed_limit_callback) {
                g_speed_limit_callback(limit);
            }
        }
    }

    // Send OK response
    httpd_resp_set_type(req, "text/plain");
    httpd_resp_send(req, "OK", 2);
    return ESP_OK;
}

/**
 * @brief HTTP GET handler for telemetry data
 */
static esp_err_t telemetry_handler(httpd_req_t *req)
{
    // Create JSON response with telemetry
    char json[120];
    snprintf(json, sizeof(json), 
             "{\"battery\":%d,\"speed\":%.1f,\"signal\":%d,\"sensors\":[%u,%u,%u]}",
             g_battery, g_speed, g_signal, g_dist_left, g_dist_center, g_dist_right);
    
    httpd_resp_set_type(req, "application/json");
    httpd_resp_send(req, json, strlen(json));
    return ESP_OK;
}

/**
 * @brief Start HTTP server
 */
static httpd_handle_t start_webserver(void)
{
    httpd_handle_t server = NULL;
    httpd_config_t config = HTTPD_DEFAULT_CONFIG();
    config.server_port = g_config.server_port;
    config.ctrl_port = 32768;
    config.max_open_sockets = 7;
    config.lru_purge_enable = true;

    ESP_LOGI(TAG, "Starting HTTP server on port: '%d'", config.server_port);
    if (httpd_start(&server, &config) == ESP_OK) {
        // Register URI handlers
        httpd_uri_t index_uri = {
            .uri       = "/",
            .method    = HTTP_GET,
            .handler   = index_handler,
            .user_ctx  = NULL
        };
        httpd_register_uri_handler(server, &index_uri);

        httpd_uri_t css_uri = {
            .uri       = "/style.css",
            .method    = HTTP_GET,
            .handler   = css_handler,
            .user_ctx  = NULL
        };
        httpd_register_uri_handler(server, &css_uri);

        httpd_uri_t js_uri = {
            .uri       = "/app.js",
            .method    = HTTP_GET,
            .handler   = js_handler,
            .user_ctx  = NULL
        };
        httpd_register_uri_handler(server, &js_uri);

        httpd_uri_t command_uri = {
            .uri       = "/command",
            .method    = HTTP_POST,
            .handler   = command_handler,
            .user_ctx  = NULL
        };
        httpd_register_uri_handler(server, &command_uri);

        httpd_uri_t telemetry_uri = {
            .uri       = "/telemetry",
            .method    = HTTP_GET,
            .handler   = telemetry_handler,
            .user_ctx  = NULL
        };
        httpd_register_uri_handler(server, &telemetry_uri);

        return server;
    }

    ESP_LOGI(TAG, "Error starting server!");
    return NULL;
}

// Public API implementation

esp_err_t web_control_init(const web_control_config_t *config)
{
    if (config == NULL) {
        return ESP_ERR_INVALID_ARG;
    }

    memcpy(&g_config, config, sizeof(web_control_config_t));

    // Initialize NVS
    esp_err_t ret = nvs_flash_init();
    if (ret == ESP_ERR_NVS_NO_FREE_PAGES || ret == ESP_ERR_NVS_NEW_VERSION_FOUND) {
        ESP_ERROR_CHECK(nvs_flash_erase());
        ret = nvs_flash_init();
    }
    ESP_ERROR_CHECK(ret);

    // Initialize network interface
    ESP_ERROR_CHECK(esp_netif_init());
    ESP_ERROR_CHECK(esp_event_loop_create_default());

    // Initialize WiFi
    ESP_ERROR_CHECK(wifi_init_softap());

    // Create timeout timer
    const esp_timer_create_args_t timer_args = {
        .callback = &motor_timeout_callback,
        .name = "motor_timeout"
    };
    ESP_ERROR_CHECK(esp_timer_create(&timer_args, &g_timeout_timer));
    ESP_ERROR_CHECK(esp_timer_start_periodic(g_timeout_timer, g_config.motor_timeout_ms * 1000));

    ESP_LOGI(TAG, "Web control initialized");
    ESP_LOGI(TAG, "Connect to WiFi: SSID='%s' Password='%s'", g_config.wifi_ssid, g_config.wifi_password);
    ESP_LOGI(TAG, "Then open browser to: http://192.168.4.1");

    return ESP_OK;
}

esp_err_t web_control_start(void)
{
    server = start_webserver();
    if (server == NULL) {
        return ESP_FAIL;
    }
    return ESP_OK;
}

esp_err_t web_control_stop(void)
{
    if (server) {
        httpd_stop(server);
        server = NULL;
    }
    return ESP_OK;
}

esp_err_t web_control_set_motor_callback(web_control_motor_callback_t callback)
{
    g_motor_callback = callback;
    return ESP_OK;
}

esp_err_t web_control_set_mode_callback(web_control_mode_callback_t callback)
{
    g_mode_callback = callback;
    return ESP_OK;
}

esp_err_t web_control_set_speed_limit_callback(web_control_speed_limit_callback_t callback)
{
    g_speed_limit_callback = callback;
    return ESP_OK;
}

bool web_control_is_connected(void)
{
    return g_client_connected;
}

esp_err_t web_control_send_telemetry(uint8_t battery_percent, float speed_kmh, uint8_t signal_strength, uint16_t dist_left, uint16_t dist_center, uint16_t dist_right)
{
    g_battery = battery_percent;
    g_speed = speed_kmh;
    g_signal = signal_strength;
    g_dist_left = dist_left;
    g_dist_center = dist_center;
    g_dist_right = dist_right;
    return ESP_OK;
}
