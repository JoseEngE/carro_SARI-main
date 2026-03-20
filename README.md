# RC Car Control System - ESP32-S3

Sistema de control para carro RC con Ackermann steering (tracción trasera, dirección delantera) usando ESP32-S3, drivers MX1508, y control web desde móvil.

## 🎯 Características

- ✅ **Control de motores PWM** con MX1508
- ✅ **Dirección con Servomotor** (MG90S)
- ✅ **Control web móvil** - Controla desde tu teléfono
- ✅ **Multi-touch** - Joystick dual independiente
- ✅ **Control de Velocidad** - Slider ajustable (10-100%)
- ✅ **Interfaz minimalista** - Diseño limpio y responsive
- ✅ **WiFi Access Point** - El ESP32 crea su propia red
- ✅ **Arquitectura basada en componentes** modular y reutilizable
- ✅ **Sistema de seguridad** con timeout automático

## 📁 Estructura del Proyecto

```
sensor/
├── components/
│   ├── motor_control/          # Control PWM de motores MX1508
│   │   ├── include/
│   │   │   └── motor_control.h
│   │   ├── src/
│   │   │   └── motor_control.c
│   │   └── CMakeLists.txt
│   ├── servo_control/          # Control PWM de servomotor
│   │   ├── include/
│   │   │   └── servo_control.h
│   │   ├── src/
│   │   │   └── servo_control.c
│   │   └── CMakeLists.txt
│   ├── web_control/            # Control web desde móvil
│   │   ├── include/
│   │   │   └── web_control.h
│   │   ├── src/
│   │   │   └── web_control.c
│   │   ├── www/                # Interfaz web embebida
│   │   │   ├── index.html
│   │   │   ├── style.css
│   │   │   └── app.js
│   │   └── CMakeLists.txt
├── main/
│   ├── main.c                  # Aplicación principal (web control)
│   └── CMakeLists.txt
├── examples/
│   ├── motor_test/             # Ejemplo de prueba de motores
│   └── web_control/            # Ejemplo de control web
├── MOTOR_TEST_GUIDE.md         # Guía de prueba de motores
├── WEB_CONTROL_GUIDE.md        # Guía de control web
├── WEB_CONTROL_TECHNICAL.md    # Documentación técnica web
└── README.md
```

## 🔧 Hardware

### ESP32-S3
- **Placa:** ESP32-S3 DevKit
- **Flash:** 2MB mínimo
- **RAM:** 512KB

### Motores y Actuadores
- **Driver:** MX1508 (x1) para motor de tracción
- **Motor tracción:** Conectado a GPIO 7, 8
- **Servo dirección:** MG90S conectado a GPIO 9
- **Alimentación motores:** 5V (separada del ESP32)
- **PWM Tracción:** 1000 Hz
- **PWM Servo:** 50 Hz

## 🚀 Inicio Rápido

### 1. Requisitos

- ESP-IDF v5.5.1
- Python 3.11
- Cable USB para programación

### 2. Configuración del Entorno

```bash
# Configurar ESP-IDF
$env:IDF_PATH = 'C:\Users\joseg\esp\v5.5.1\esp-idf'
```

### 3. Compilar y Flashear

```bash
cd sensor
idf.py build
idf.py -p COM4 flash monitor
```

### 4. Conectar desde el Móvil

1. **Conecta a WiFi:**
   - SSID: `RC_Car_Control`
   - Password: `rccar123`

2. **Abre el navegador:**
   - URL: `http://192.168.4.1`

3. **Controla el carro:**
   - Joystick izquierdo: Dirección
   - Joystick derecho: Velocidad
   - Botón rojo: STOP de emergencia

## 📖 Componentes

### Motor Control

Control PWM para motor DC de tracción con MX1508.

**Características:**
- Control de velocidad -100% a +100%
- Thread-safe con mutexes

### Servo Control

Control PWM para servomotor de dirección (MG90S).

**Características:**
- Control de ángulo preciso
- Mapeo de ángulos configurables (min/max)
- Frecuencia 50Hz estándar

**Ejemplo de uso:**

```c
#include "motor_control.h"
#include "servo_control.h"

// Configurar motor tracción
motor_config_t drive_config = {
    .in1_pin = GPIO_NUM_7,
    .in2_pin = GPIO_NUM_8,
    .pwm_freq_hz = 1000,
    .timer = LEDC_TIMER_0,
    .channel_a = LEDC_CHANNEL_0,
    .channel_b = LEDC_CHANNEL_1
};

// Configurar servo
servo_config_t servo_cfg = {
    .gpio_num = GPIO_NUM_9,
    .timer_number = LEDC_TIMER_1,
    .channel_number = LEDC_CHANNEL_2,
    .min_pulse_width_us = 500,
    .max_pulse_width_us = 2400,
    .frequency = 50,
    .min_angle = 60.0f,
    .max_angle = 120.0f,
    .initial_angle = 90.0f
};

// Inicializar
motor_control_init(&drive_config, NULL);
servo_init(&servo_cfg);

// Controlar
motor_drive_forward(50);     // Adelante 50%
servo_set_angle(80.0f);      // Girar servo
```

### Web Control

Control remoto desde móvil con interfaz web minimalista.

**Características:**
- WiFi Access Point integrado
- Interfaz responsive optimizada para móvil
- Control dual joystick (throttle + steering)
- Telemetría en tiempo real (batería, velocidad, señal)
- HTTP polling para compatibilidad
- Sistema de seguridad con timeout (500ms)
- Botón de emergencia

**Ejemplo de uso:**

```c
#include "web_control.h"

// Configurar web control
web_control_config_t web_config = WEB_CONTROL_DEFAULT_CONFIG();
web_control_init(&web_config);

// Registrar callback de motor
void motor_callback(int8_t throttle, int8_t steering) {
    // Control Motor Tracción
    if (throttle > 5) {
        motor_drive_forward(throttle);
    } else if (throttle < -5) {
        motor_drive_backward(-throttle);
    } else {
        motor_drive_stop();
    }
    
    // Control Servo Dirección
    // Mapear -100..100 a grados (ej. 60..120)
    float angle = 90.0f + (steering * 0.3f); 
    servo_set_angle(angle);
}
web_control_set_motor_callback(motor_callback);

// Iniciar servidor
web_control_start();

// Actualizar telemetría
web_control_send_telemetry(87, 0.0, 100);
```

## 🎮 Aplicación Principal

El `main.c` actual implementa control web completo:

1. **Inicialización de motores** - Configura drivers MX1508
2. **Inicialización de web control** - Crea WiFi AP y servidor HTTP
3. **Registro de callback** - Conecta comandos web con motores
4. **Loop de telemetría** - Actualiza datos en tiempo real

Para probar solo los motores, usa el ejemplo en `examples/motor_test/`.

## 📝 Documentación Adicional

- [`MOTOR_TEST_GUIDE.md`](MOTOR_TEST_GUIDE.md) - Guía del test de motores
- [`WEB_CONTROL_GUIDE.md`](WEB_CONTROL_GUIDE.md) - Guía de control web
- [`WEB_CONTROL_TECHNICAL.md`](WEB_CONTROL_TECHNICAL.md) - Documentación técnica web

## ⚠️ Notas Importantes

### Alimentación de Motores

**CRÍTICO:** Los motores MX1508 deben tener alimentación separada:

```
ESP32 (USB 5V) ──────┐
                     │
MX1508 (5V) ──┼── GND común
                     │
Motores ─────────────┘
```

- **NO** alimentar motores desde el ESP32
- **SÍ** conectar GND común entre ESP32 y MX1508
- **SÍ** usar alimentación de 5V para motores

### Frecuencia PWM

- Usar **1000 Hz** para evitar resets del watchdog
- No usar frecuencias >3000 Hz

### Seguridad del Control Web

- ⚠️ **Timeout automático** - Motores se detienen si no hay comandos por 500ms
- ⚠️ **Botón de emergencia** - Siempre accesible en la interfaz
- ⚠️ **Validación de comandos** - Valores limitados a rangos seguros
- ⚠️ **Autenticación WiFi** - Contraseña WPA2-PSK

## 🌐 Control Web - Características

### Interfaz Minimalista

- **Diseño limpio** - Tema claro con sombras sutiles
- **Joysticks virtuales** - Control preciso con multi-touch nativo
- **Speed Limiter** - Slider para ajustar velocidad máxima
- **Responsive** - Adaptable a diferentes tamaños de pantalla
- **Indicadores en tiempo real** - Batería, velocidad, señal, latencia

### Protocolo de Comunicación

**Comandos (HTTP POST):**
```
POST /command
Body: [msg_type, throttle, steering, checksum]
```

**Telemetría (HTTP GET):**
```
GET /telemetry
Response: {"battery":87, "speed":0.0, "signal":100}
```

### Rendimiento

- **Frecuencia de comandos**: 50Hz (cada 20ms)
- **Actualización de telemetría**: 10Hz (cada 100ms)
- **Latencia típica**: 80-120ms
- **Timeout de seguridad**: 500ms

## 🔮 Desarrollo Futuro

- [ ] WebSocket para menor latencia (requiere ESP-IDF 5.3+)
- [ ] Video streaming con ESP32-CAM
- [ ] Modo autónomo incorporando sensores
- [ ] Telemetría avanzada (temperatura, corriente, voltaje)
- [ ] Grabación y replay de rutas
- [ ] Múltiples perfiles de velocidad
- [ ] Control por voz
- [ ] Aplicación móvil nativa

## 🐛 Solución de Problemas

### Web Control

**No puedo ver la red WiFi:**
- Verifica que el ESP32 esté encendido
- Revisa el monitor serial: debe decir "WiFi AP started"

**La página no carga:**
- Usa exactamente `http://192.168.4.1` (no https)
- Intenta en modo incógnito
- Limpia la caché del navegador

**Alta latencia (>200ms):**
- Acércate más al ESP32
- Reduce interferencias WiFi
- Cambia el canal WiFi en la configuración

### Motores

**Motor no se mueve:**
- Verifica conexiones de GPIOs
- Comprueba alimentación de 5V
- Revisa GND común

**Motor gira al revés:**
- Intercambia cables IN1 e IN2 del motor afectado

Ver [`MOTOR_TEST_GUIDE.md`](MOTOR_TEST_GUIDE.md) para más detalles.

## 📄 Licencia

Proyecto educativo - ESP32-S3 RC Car Control System

---

**Versión:** 2.0  
**Hardware:** ESP32-S3 + MX1508  
**Firmware:** ESP-IDF v5.5.1  
**Nuevas características:** Control web móvil con interfaz minimalista
