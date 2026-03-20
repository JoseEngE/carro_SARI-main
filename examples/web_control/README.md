# Web Control Example

Este ejemplo demuestra cómo usar el componente `web_control` para controlar el RC car desde un teléfono móvil.

## 🎯 Características

- Control web desde móvil con interfaz minimalista
- WiFi Access Point integrado
- Joysticks virtuales para throttle y steering
- Telemetría en tiempo real
- Sistema de seguridad con timeout
- Botón de emergencia

## 🔧 Hardware Requerido

- ESP32-S3 DevKit
- 2x MX1508 motor drivers
- 2x DC motors (tracción y dirección)
- Fuente de alimentación 5V para motores
- Cables de conexión

## 📋 Conexiones

### Motor de Tracción (Trasero)
- IN1 → GPIO 7
- IN2 → GPIO 8

### Motor de Dirección (Delantero)
- IN1 → GPIO 9
- IN2 → GPIO 10

### Alimentación
- VM (MX1508) → 5V
- GND → GND común con ESP32

## 🚀 Cómo Usar

### 1. Compilar y Flashear

```bash
cd examples/web_control
idf.py build
idf.py -p COM4 flash monitor
```

### 2. Conectar desde el Móvil

1. En tu teléfono, busca la red WiFi: **RC_Car_Control**
2. Conéctate con la contraseña: **rccar123**
3. Abre el navegador y ve a: **http://192.168.4.1**

### 3. Controlar el Carro

- **Joystick Izquierdo**: Control de dirección (steering)
  - Izquierda/Derecha para girar
  
- **Joystick Derecho**: Control de velocidad (throttle)
  - Arriba para avanzar
  - Abajo para retroceder
  
- **Botón STOP**: Detención de emergencia

## 📊 Salida Esperada

```
╔════════════════════════════════════════╗
║    RC Car Web Control System           ║
╚════════════════════════════════════════╝

Initializing motors...
✓ Motors initialized
Initializing web control...
WiFi AP started. SSID:RC_Car_Control password:rccar123 channel:1
✓ Web control initialized
Starting web server...
Starting HTTP server on port: '80'
✓ Web server started

╔════════════════════════════════════════╗
║  Web Control Ready!                    ║
╚════════════════════════════════════════╝

📱 Connect your phone to WiFi:
   SSID: RC_Car_Control
   Password: rccar123

🌐 Open browser and go to:
   http://192.168.4.1
```

## ⚙️ Configuración

### Cambiar Credenciales WiFi

Edita `main.c` y modifica:

```c
web_control_config_t web_config = {
    .wifi_ssid = "MiCarroRC",
    .wifi_password = "mipassword123",
    .wifi_channel = 1,
    .max_connections = 4,
    .server_port = 80,
    .motor_timeout_ms = 500
};
```

### Ajustar Zona Muerta de Joysticks

En la función `motor_callback()`, cambia el valor de 5:

```c
// Zona muerta más grande (10%)
if (throttle > 10) {
    motor_drive_forward(throttle);
}
```

## 🔒 Seguridad

Este ejemplo incluye varias características de seguridad:

1. **Timeout Automático**: Si no se reciben comandos por 500ms, los motores se detienen
2. **Validación de Comandos**: Todos los valores se limitan a -100 a +100
3. **Botón de Emergencia**: Detención inmediata desde la interfaz
4. **Autenticación WiFi**: WPA2-PSK para proteger el acceso

## 📝 Notas

- ⚠️ **Importante**: La alimentación de los motores debe ser separada del ESP32
- ⚠️ Asegúrate de que el GND sea común entre ESP32 y MX1508
- ⚠️ Prueba primero con el carro elevado antes de usarlo en el suelo
- ⚠️ Mantén el botón STOP siempre accesible

## 🐛 Solución de Problemas

### No veo la red WiFi
- Verifica que el ESP32 esté encendido
- Revisa el monitor serial: debe decir "WiFi AP started"

### La página no carga
- Usa exactamente `http://192.168.4.1` (no https)
- Intenta en modo incógnito del navegador

### Los motores no responden
- Verifica las conexiones de los GPIOs
- Comprueba la alimentación de 5V
- Revisa el monitor serial para ver si llegan comandos

### Alta latencia
- Acércate más al ESP32
- Reduce interferencias WiFi
- Cierra otras apps en el teléfono

## 📚 Documentación Relacionada

- [WEB_CONTROL_GUIDE.md](../../WEB_CONTROL_GUIDE.md) - Guía completa de usuario
- [WEB_CONTROL_TECHNICAL.md](../../WEB_CONTROL_TECHNICAL.md) - Documentación técnica
- [MOTOR_TEST_GUIDE.md](../../MOTOR_TEST_GUIDE.md) - Guía de prueba de motores

## 🔄 Próximos Pasos

Después de probar este ejemplo, puedes:

1. Añadir modo autónomo
3. Implementar telemetría real (batería, velocidad)
4. Personalizar la interfaz web (colores, diseño)

---

**Ejemplo**: Web Control  
**Componentes**: motor_control, web_control  
**Dificultad**: Intermedio
