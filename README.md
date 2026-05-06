# RC Car Control System - ESP32-S3

<table>
  <tr>
    <td width="40%">
      <!-- Pega aquí la URL cruda del video que GitHub generó al arrastrarlo -->
      <video src="https://github.com/user-attachments/assets/2d20ece8-12e0-4c50-b98b-551186aeced8" controls="controls" width="100%"></video>
    </td>
    <td width="60%" valign="top">
      <h3>RC Car Control System - ESP32-S3</h3>
      <p>Sistema avanzado de control para un carro RC con dirección tipo Ackermann (tracción trasera, dirección delantera). Basado en el ESP32-S3, este proyecto integra control web responsivo, gestión de múltiples sensores de distancia (VL53L0X) y un modo de conducción autónoma (Wall Follower).</p>
      
## 🎯 Características Principales

- ✅ **Control Web Remoto**: Interfaz móvil minimalista, control dual con joysticks virtuales (Multi-touch).
- ✅ **Modos de Operación**: 
  - *Manual*: Control total sobre la tracción y dirección desde el móvil.
  - *Autónomo*: Navegación inteligente evitando obstáculos y siguiendo paredes.
- ✅ **Tracción y Dirección**: Control PWM para motores DC (MX1508) y servomotor (MG90S) para una dirección precisa.
- ✅ **Sistema de Sensores ToF**: Integración de 3 sensores láser VL53L0X (Izquierda, Centro, Derecha) mediante un bus I2C común.
- ✅ **Telemetría en Tiempo Real**: Envío de lecturas de sensores, estado de batería simulado, velocidad y calidad de señal a la interfaz web.
- ✅ **WiFi Access Point**: El ESP32 crea su propia red aislada de baja latencia.
- ✅ **Arquitectura Modular**: Código altamente estructurado en componentes independientes de ESP-IDF.
      
    </td>
  </tr>
</table>




## 📁 Estructura del Proyecto

El código está organizado en componentes modulares siguiendo las convenciones de **ESP-IDF**:

```
carro_SARI-main/
├── components/                 # Módulos reutilizables del sistema
│   ├── motor_control/          # Control PWM de motores DC (Driver MX1508)
│   ├── servo_control/          # Control PWM del servomotor de dirección
│   ├── sensor_manager/         # Inicialización I2C y multiplexado (Pines XSHUT) de múltiples VL53L0X
│   ├── vl53l0x/                # API/Driver base para el sensor de distancia VL53L0X
│   ├── wall_follower/          # Lógica del algoritmo de conducción autónoma
│   └── web_control/            # Servidor HTTP, WebSockets/Polling y UI (HTML/CSS/JS)
├── main/                       # Aplicación Principal
│   ├── main.c                  # Punto de entrada, tareas FreeRTOS, configuración global
│   └── CMakeLists.txt
├── examples/                   # Códigos de prueba para componentes individuales
├── HARDWARE_GUIDE.md           # Guía detallada de conexiones electrónicas
├── WEB_CONTROL_GUIDE.md        # Documentación de uso de la interfaz web
├── WEB_CONTROL_TECHNICAL.md    # Documentación técnica del servidor y frontend
└── README.md                   # Este archivo
```

---

## 🔧 Hardware y Conexiones

### Componentes Principales
- **Microcontrolador**: ESP32-S3 DevKit
- **Driver de Motor**: MX1508 para el motor de tracción trasero
- **Motor de Dirección**: Servomotor MG90S
- **Sensores de Distancia**: 3x VL53L0X (Time-of-Flight)

### Asignación de Pines (Definidos en `main.c`)
- **Motor de Tracción (MX1508)**: GPIO 2 (IN1), GPIO 42 (IN2). Frecuencia: 1000 Hz.
- **Servo de Dirección (MG90S)**: GPIO 1. Frecuencia: 50 Hz.
- **Bus I2C Sensores (VL53L0X)**: GPIO 10 (SDA), GPIO 9 (SCL). Frecuencia: 400kHz.
- **Pines XSHUT (Multiplexado Sensores)**: 
  - Sensor Derecha: GPIO 14
  - Sensor Centro: GPIO 13
  - Sensor Izquierda: GPIO 12


---

## ⚙️ Cómo Funciona el Sistema Global

El flujo del sistema corre sobre **FreeRTOS** integrado en ESP-IDF, dividiendo las responsabilidades en distintas tareas paralelas:

1. **Inicialización (`app_main`)**:
   - Inicia los drivers (Motores, Servos, Web Server).
   - Inicia el **Sensor Manager**, asignando direcciones I2C dinámicas a cada sensor VL53L0X activando secuencialmente sus pines *XSHUT*.
   - Comienza a publicar datos de telemetría hacia la interfaz web.

2. **Control Manual (Interrupt-Driven)**:
   - Cuando el usuario usa los joysticks en la web, se envían comandos HTTP que disparan callbacks (`motor_callback`).
   - El ESP32 traduce comandos de tracción y mapea un rango de dirección web (-100 a 100) al rango físico seguro del servomotor (ej. 41º a 75º).

3. **Modo Autónomo (`auto_navigation_task`)**:
   - Es una tarea FreeRTOS permanente. 
   - Constantemente lee datos de los 3 sensores a través de `sensor_manager_read_all()`.
   - Si el modo autónomo es activado (vía web control callback `mode_callback`), delega las decisiones al componente `wall_follower`.
   - `wall_follower_process` evalúa las distancias izquierda, centro y derecha para emitir los nuevos niveles de velocidad y dirección (esquiva obstáculos o sigue paredes). Finalmente, estos valores se inyectan a los motores.

4. **Telemetría Transparente**:
   - Independientemente del modo, el bucle principal en `app_main` empuja (push) frecuentemente las distancias de los sensores globales y la información de estado hacia `web_control`, para actualizar la pestaña de telemetría en el navegador web.

---

## 🚀 Inicio Rápido

### Requisitos Previos
- **ESP-IDF v5.5.1** configurado.
- Entorno configurado (Python 3.11, etc.).

### 1. Compilar y Grabar
Abre tu terminal (o ESP-IDF de Visual Studio Code) en el directorio raíz del proyecto:
```bash
idf.py build
idf.py -p COMx flash monitor  # Sustituye COMx por tu puerto (ej. COM4)
```

### 2. Conectar a la Interfaz Web
1. Desde tu celular o PC, busca redes WiFi y conéctate a:
   - **SSID**: `RC_Car_Control`
   - **Contraseña**: `rccar123`
2. Abre cualquier navegador moderno y dirígete a:
   - **`http://192.168.4.1`**

### 3. Operación
- Usa los **Joysticks** para conducción manual.
- Usa los **Controles Auxiliares / Botón Auto** para alternar entre el control manual y la conducción automática basada en sensores.
- Ajusta el *Speed Limit* para domar la agresividad del coche de forma global.

---

## 📄 Notas Adicionales y Futuras Mejoras

- Verifica frecuentemente el documento [`HARDWARE_GUIDE.md`](HARDWARE_GUIDE.md) en caso de experimentar problemas de comunicación I2C o bloqueos en los motores.
- **Rendimiento**: El polling de los 3 sensores I2C utiliza el modo "High Precision", lo cual genera un ligero retraso de unos milisegundos compensado en los delays de FreeRTOS.
- **Trabajo Futuro**: Mover las comunicaciones Web a WebSockets por defecto (si el ESP-IDF lo permite sin overhead excesivo) y añadir algoritmos PID más complejos dentro de `wall_follower` para suavizar cruces de esquinas cerradas.
