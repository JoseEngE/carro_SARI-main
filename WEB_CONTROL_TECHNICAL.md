# Web Control Component - Documentación Técnica

## 📐 Arquitectura del Sistema

### Diagrama de Componentes

```
┌─────────────────────────────────────────────────┐
│                Mobile Phone                      │
│  ┌───────────────────────────────────────────┐  │
│  │         Web Browser                       │  │
│  │  ┌─────────────┐  ┌──────────────────┐   │  │
│  │  │  HTML/CSS   │  │   JavaScript     │   │  │
│  │  │  Interface  │  │   Controller     │   │  │
│  │  └─────────────┘  └────────┬─────────┘   │  │
│  │                            │             │  │
│  │                       (Speed Limiter)    │  │
│  └───────────────────────────────────────────┘  │
│  └───────────────────────────────────────────┘  │
└──────────────────┬──────────────────────────────┘
                   │ HTTP (WiFi)
                   │ POST /command (50Hz)
                   │ GET /telemetry (10Hz)
┌──────────────────▼──────────────────────────────┐
│              ESP32-S3                            │
│  ┌───────────────────────────────────────────┐  │
│  │      web_control Component                │  │
│  │  ┌─────────────┐  ┌──────────────────┐   │  │
│  │  │ WiFi AP     │  │  HTTP Server     │   │  │
│  │  │ (esp_wifi)  │  │  (esp_http_srv)  │   │  │
│  │  └─────────────┘  └──────────────────┘   │  │
│  │  ┌─────────────┐  ┌──────────────────┐   │  │
│  │  │ Timeout     │  │  Motor Callback  │   │  │
│  │  │ Timer       │  │  Handler         │   │  │
│  │  └─────────────┘  └──────────────────┘   │  │
│  └───────────────────────────────────────────┘  │
│                      │                           │
│  ┌───────────────────▼───────────────────────┐  │
│  │      motor_control Component              │  │
│  │  ┌─────────────┐  ┌──────────────────┐   │  │
│  │  │ PWM Driver  │  │  Motor Logic     │   │  │
│  │  │ (LEDC)      │  │                  │   │  │
│  │  └─────────────┘  └──────────────────┘   │  │
│  └───────────────────────────────────────────┘  │
└──────────────────┬──────────────────────────────┘
                   │ PWM Signals
┌──────────────────▼──────────────────────────────┐
│              MX1508 Drivers                      │
│  ┌─────────────┐  ┌──────────────────┐          │
│  │ Driver 1    │  │  Driver 2        │          │
│  │ (Throttle)  │  │  (Steering)      │          │
│  └─────────────┘  └──────────────────┘          │
└──────────────────┬──────────────────────────────┘
                   │ Motor Power
┌──────────────────▼──────────────────────────────┐
│                DC Motors                         │
└──────────────────────────────────────────────────┘
```

## 🔧 Estructura de Archivos

```
components/web_control/
├── include/
│   └── web_control.h          # API pública del componente
├── src/
│   └── web_control.c          # Implementación
├── www/
│   ├── index.html             # Interfaz web
│   ├── style.css              # Estilos minimalistas
│   └── app.js                 # Lógica de control
└── CMakeLists.txt             # Configuración de build
```

## 📡 API del Componente

### Estructuras de Datos

#### `web_control_config_t`

```c
typedef struct {
    const char *wifi_ssid;          // SSID del AP
    const char *wifi_password;      // Contraseña del AP
    uint8_t wifi_channel;           // Canal WiFi (1-13)
    uint8_t max_connections;        // Conexiones simultáneas
    uint16_t server_port;           // Puerto HTTP (default: 80)
    uint32_t motor_timeout_ms;      // Timeout de seguridad
} web_control_config_t;
```

#### `web_control_motor_callback_t`

```c
typedef void (*web_control_motor_callback_t)(int8_t throttle, int8_t steering);
```

### Funciones Públicas

#### `web_control_init()`

Inicializa el componente web_control.

```c
esp_err_t web_control_init(const web_control_config_t *config);
```

**Parámetros:**
- `config`: Puntero a configuración

**Retorna:**
- `ESP_OK` si éxito
- `ESP_ERR_INVALID_ARG` si config es NULL

**Ejemplo:**
```c
web_control_config_t config = WEB_CONTROL_DEFAULT_CONFIG();
esp_err_t ret = web_control_init(&config);
```

#### `web_control_start()`

Inicia el servidor HTTP.

```c
esp_err_t web_control_start(void);
```

**Retorna:**
- `ESP_OK` si el servidor inició correctamente
- `ESP_FAIL` si falló

#### `web_control_stop()`

Detiene el servidor HTTP.

```c
esp_err_t web_control_stop(void);
```

#### `web_control_set_motor_callback()`

Registra callback para comandos de motor.

```c
esp_err_t web_control_set_motor_callback(web_control_motor_callback_t callback);
```

**Parámetros:**
- `callback`: Función a llamar cuando se reciben comandos

**Ejemplo:**
```c
void motor_callback(int8_t throttle, int8_t steering) {
    if (throttle > 5) {
        motor_drive_forward(throttle);
    } else if (throttle < -5) {
        motor_drive_backward(-throttle);
    } else {
        motor_drive_stop();
    }
    
    motor_steering_set_angle(steering);
}

web_control_set_motor_callback(motor_callback);
```

#### `web_control_is_connected()`

Verifica si hay un cliente conectado.

```c
bool web_control_is_connected(void);
```

**Retorna:**
- `true` si hay cliente conectado
- `false` si no hay conexión

#### `web_control_send_telemetry()`

Actualiza datos de telemetría.

```c
esp_err_t web_control_send_telemetry(uint8_t battery_percent, 
                                      float speed_kmh, 
                                      uint8_t signal_strength);
```

**Parámetros:**
- `battery_percent`: Batería 0-100%
- `speed_kmh`: Velocidad en km/h
- `signal_strength`: Señal 0-100%

## 🌐 Protocolo HTTP

### Endpoints

#### GET `/`
Sirve la página principal (index.html)

**Respuesta:**
- Content-Type: `text/html`
- Body: HTML de la interfaz

#### GET `/style.css`
Sirve los estilos CSS

**Respuesta:**
- Content-Type: `text/css`
- Body: CSS minimalista

#### GET `/app.js`
Sirve el JavaScript de control

**Respuesta:**
- Content-Type: `application/javascript`
- Body: Lógica de control

#### POST `/command`
Recibe comandos de motor

**Request:**
- Content-Type: `application/octet-stream`
- Body: Datos binarios [msg_type, throttle, steering, checksum]

**Response:**
- Content-Type: `text/plain`
- Body: `OK`

**Formato de Comandos:**

| Byte | Descripción | Valores |
|------|-------------|---------|
| 0 | Tipo de mensaje | 0x01=Motor, 0x03=Stop |
| 1 | Throttle | -100 a +100 (signed) |
| 2 | Steering | -100 a +100 (signed) |
| 3 | Checksum | (byte1 + byte2) & 0xFF |

#### GET `/telemetry`
Obtiene datos de telemetría

**Response:**
- Content-Type: `application/json`
- Body: JSON con telemetría

**Formato JSON:**
```json
{
  "battery": 87,
  "speed": 0.0,
  "signal": 100
}
```

## ⚡ Flujo de Datos

### Secuencia de Inicialización

```
1. main.c llama web_control_init()
   ├─> Inicializa NVS
   ├─> Inicializa esp_netif
   ├─> Configura WiFi en modo AP
   ├─> Crea timer de timeout
   └─> Registra event handlers

2. main.c llama web_control_start()
   ├─> Inicia servidor HTTP
   ├─> Registra handlers de URI
   └─> Embebe archivos HTML/CSS/JS

3. main.c llama web_control_set_motor_callback()
   └─> Registra función de callback

4. Sistema listo para recibir conexiones
```

### Secuencia de Control

```
1. Usuario abre navegador → GET /
   └─> Servidor envía index.html + CSS + JS

2. JavaScript inicia polling
   ├─> POST /command cada 20ms (50Hz)
   │   ├─> Envía [0x01, throttle, steering, checksum]
   │   └─> Servidor llama motor_callback()
   │
   └─> GET /telemetry cada 100ms (10Hz)
       └─> Servidor envía JSON con datos

3. Timer de timeout verifica cada 500ms
   └─> Si no hay comandos → detiene motores
```

## 🔒 Características de Seguridad

### 1. Timeout Automático

```c
static void motor_timeout_callback(void* arg)
{
    int64_t now = esp_timer_get_time() / 1000;
    if (g_client_connected && 
        (now - g_last_command_time > g_config.motor_timeout_ms)) {
        // Detener motores
        if (g_motor_callback) {
            g_motor_callback(0, 0);
        }
    }
}
```

### 2. Control de Velocidad (Cliente)

Implementado en `app.js` mediante un slider en la interfaz:

```javascript
// Aplicar límite de velocidad
const limitedThrottle = Math.round((throttleValue * maxSpeedPercent) / 100);
```

### 3. Validación de Comandos (Servidor)

```c
// Limitar valores
if (throttle < -100) throttle = -100;
if (throttle > 100) throttle = 100;
if (steering < -100) steering = -100;
if (steering > 100) steering = 100;
```

### 4. Soporte Multi-Touch

Implementado usando `touch.identifier` para rastrear dedos independientes para Throttle y Steering, permitiendo control simultáneo sin conflictos.

### 3. Autenticación WiFi

- WPA2-PSK por defecto
- Contraseña configurable
- Máximo de conexiones limitado

## 📊 Rendimiento

### Métricas de Latencia

| Operación | Latencia Típica | Máxima Aceptable |
|-----------|----------------|------------------|
| Comando POST | 80-120ms | 200ms |
| Telemetría GET | 50-80ms | 150ms |
| Timeout Detection | 500ms | 1000ms |

### Consumo de Recursos

| Recurso | Uso |
|---------|-----|
| RAM | ~30KB |
| Flash | ~20KB (HTML/CSS/JS embebidos) |
| CPU | ~5% (polling activo) |
| WiFi | ~50mA (AP mode) |

### Frecuencias de Actualización

- **Comandos**: 50Hz (cada 20ms)
- **Telemetría**: 10Hz (cada 100ms)
- **Heartbeat**: 1Hz (cada 1000ms)
- **Timeout check**: 2Hz (cada 500ms)

## 🛠️ Integración con motor_control

### Callback de Motor

```c
void motor_callback(int8_t throttle, int8_t steering)
{
    // Throttle: -100 (atrás) a +100 (adelante)
    // Steering: -100 (izquierda) a +100 (derecha)
    
    // Zona muerta de 5% para evitar drift
    if (throttle > 5) {
        motor_drive_forward(throttle);
    } else if (throttle < -5) {
        motor_drive_backward(-throttle);
    } else {
        motor_drive_stop();
    }
    
    // Aplicar dirección
    if (steering > 5 || steering < -5) {
        motor_steering_set_angle(steering);
    } else {
        motor_steering_center();
    }
}
```

## 🔄 Ciclo de Vida

```
┌─────────────┐
│   INIT      │ web_control_init()
└──────┬──────┘
       │
       ▼
┌─────────────┐
│   READY     │ web_control_start()
└──────┬──────┘
       │
       ▼
┌─────────────┐
│  RUNNING    │◄─── Recibe comandos
└──────┬──────┘    Envía telemetría
       │           Verifica timeout
       ▼
┌─────────────┐
│  STOPPED    │ web_control_stop()
└─────────────┘
```

## 📝 Notas de Implementación

### HTTP Polling vs WebSocket

**Por qué HTTP Polling:**
- Compatible con ESP-IDF 5.5.1
- Más simple de implementar
- No requiere API de WebSocket
- Latencia aceptable (~100ms)

**Desventajas:**
- Mayor overhead de red
- Latencia ligeramente mayor que WebSocket
- Más consumo de CPU

### Archivos Embebidos

Los archivos HTML/CSS/JS se embeben en el firmware usando `EMBED_FILES` en CMakeLists.txt:

```cmake
EMBED_FILES "www/index.html" "www/style.css" "www/app.js"
```

Esto permite:
- No necesitar sistema de archivos
- Acceso rápido a archivos
- Firmware autocontenido

## 🐛 Debugging

### Logs Importantes

```c
ESP_LOGI(TAG, "WiFi AP started. SSID:%s", ssid);
ESP_LOGI(TAG, "Station joined, AID=%d", aid);
ESP_LOGI(TAG, "Motor command: throttle=%d, steering=%d", t, s);
ESP_LOGW(TAG, "Motor command timeout - stopping motors");
```

### Monitoreo

```bash
idf.py monitor
```

Buscar:
- "WiFi AP started" - WiFi OK
- "Starting HTTP server" - Servidor OK
- "Station joined" - Cliente conectado
- "Motor command" - Comandos llegando

---

**Versión**: 1.0  
**Autor**: Web Control Component  
**Fecha**: 2026-01-11
