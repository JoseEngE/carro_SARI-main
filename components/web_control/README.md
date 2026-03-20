# Web Control Component

Componente para el control remoto del RC Car mediante una interfaz web accesible desde cualquier smartphone.

## 📋 Características

- **WiFi ACcess Point**: Crea su propia red para conexión directa y baja latencia.
- **Interfaz Minimalista**: Joystick virtual dual (Throttle + Steering) optimizado para móviles.
- **Multi-Touch**: Control independiente de dirección y velocidad simultáneamente.
- **Control de Velocidad**: Slider ajustable para limitar la velocidad máxima (10-100%).
- **Telemetría**: Visualización en tiempo real de batería, velocidad y señal.
- **Seguridad**: Timeout automático y botón de parada de emergencia.

## 🛠️ Dependencias

- `esp_wifi`: Gestión de conexión WiFi.
- `esp_http_server`: Servidor web para la interfaz y API.
- `esp_timer`: Gestión de timeouts y telemetría.

## 💻 Uso

### Inicialización Básica

```c
#include "web_control.h"

// Configuración por defecto
web_control_config_t config = WEB_CONTROL_DEFAULT_CONFIG();
ESP_ERROR_CHECK(web_control_init(&config));

// Registrar callback de control
void motor_callback(int8_t throttle, int8_t steering) {
    // Aplicar lógica de motores aquí
}
web_control_set_motor_callback(motor_callback);

// Iniciar servidor
ESP_ERROR_CHECK(web_control_start());
```

## ⚙️ Configuración Web

La interfaz web permite ajustar:
- **Max Speed**: Limita la potencia máxima enviada a los motores (seguridad/aprendizaje).

## 📡 API HTTP

- `POST /command`: Envío de comandos de control (binario).
- `GET /telemetry`: Recepción de estado del vehículo (JSON).

Para más detalles técnicos, consultar `WEB_CONTROL_TECHNICAL.md`.
