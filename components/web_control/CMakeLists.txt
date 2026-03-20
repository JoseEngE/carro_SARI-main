idf_component_register(
    SRCS "src/web_control.c"
    INCLUDE_DIRS "include"
    REQUIRES esp_http_server esp_wifi nvs_flash esp_netif esp_event esp_timer motor_control
    EMBED_FILES "www/index.html" "www/style.css" "www/app.js"
)
