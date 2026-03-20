# Motor Control Component

Componente para el control de motores DC usando drivers MX1508, diseñado para vehículos tipo RC con dirección Ackermann (tracción trasera + dirección delantera).

## 📋 Características

- **Control PWM Independiente**: Gestión precisa de velocidad usando LEDC.
- **Dirección Ackermann**: Soporte nativo para vehículos con motor de dirección.
- **Thread-Safe**: Protegido por Mutex para acceso seguro desde múltiples tareas.
- **Control de Tiempo**: Uso de `esp_timer` para funciones de temporización precisas (kick-and-hold para dirección).

## 🛠️ Dependencias

Este componente requiere los siguientes componentes del IDF:
- `driver` (LEDC, GPIO)
- `esp_timer` (Temporización)
- `freertos` (Mutex, Tasks)
- `log` (Logging)

## 🔌 Configuración Hardware

### Driver MX1508
El componente está diseñado para funcionar con puentes H duales como el MX1508 o L298N.

#### Esquema de Conexión Típico
- **Motor Tracción (Drive):**
  - IN1 -> GPIO (Configurable)
  - IN2 -> GPIO (Configurable)
- **Motor Dirección (Steering):**
  - IN1 -> GPIO (Configurable)
  - IN2 -> GPIO (Configurable)

## 💻 Uso

### Inicialización

```c
#include "motor_control.h"

// Configuración Motor Tracción
motor_config_t drive_cfg = {
    .in1_pin = GPIO_NUM_7,
    .in2_pin = GPIO_NUM_8,
    .pwm_freq_hz = 1000,
    .timer = LEDC_TIMER_0,
    .channel_a = LEDC_CHANNEL_0,
    .channel_b = LEDC_CHANNEL_1
};

// Configuración Motor Dirección
motor_config_t steer_cfg = {
    .in1_pin = GPIO_NUM_9,
    .in2_pin = GPIO_NUM_10,
    .pwm_freq_hz = 1000,
    .timer = LEDC_TIMER_1,
    .channel_a = LEDC_CHANNEL_2,
    .channel_b = LEDC_CHANNEL_3
};

// Inicializar
ESP_ERROR_CHECK(motor_control_init(&drive_cfg, &steer_cfg));
```

### Control Básico

```c
// Avanzar al 50% de velocidad
motor_drive_forward(50);

// Girar dirección 30 grados a la izquierda (aprox)
motor_steering_set_angle(-30);

// Combinado: Girar a la derecha mientras se retrocede
motor_move_backward(40, 60);

// Parada de emergencia
motor_stop_all();
```

## ⚙️ API Reference

Ver `include/motor_control.h` para la documentación completa de funciones y tipos.
