# Web Control Component - Guía de Usuario

## 🎯 Descripción

El componente `web_control` permite controlar el RC car desde un teléfono móvil a través de una interfaz web minimalista. El ESP32 crea un punto de acceso WiFi y sirve una aplicación web que funciona como control remoto.

## 🌐 Características

- ✅ **WiFi Access Point** - El ESP32 crea su propia red WiFi
- ✅ **Interfaz minimalista** - Diseño limpio y fácil de usar
- ✅ **Control dual joystick** - Throttle (velocidad) y Steering (dirección)
- ✅ **Multi-touch** - Control simultáneo e independiente de ambos joysticks
- ✅ **Control de Velocidad** - Slider para limitar la velocidad máxima
- ✅ **Botón de emergencia** - Detención inmediata
- ✅ **Telemetría en tiempo real** - Batería, velocidad, señal
- ✅ **HTTP Polling** - Compatible con ESP-IDF 5.5.1
- ✅ **Baja latencia** - ~100ms de respuesta
- ✅ **Timeout de seguridad** - Detiene motores si pierde conexión

## 📱 Cómo Usar

### Paso 1: Flashear el ESP32

```bash
idf.py build
idf.py -p COM4 flash monitor
```

### Paso 2: Conectar el Teléfono

1. En tu teléfono, ve a **Configuración → WiFi**
2. Busca la red: **RC_Car_Control**
3. Conéctate con la contraseña: **rccar123**
4. Espera a que se conecte (IP: 192.168.4.x)

### Paso 3: Abrir la Interfaz Web

1. Abre el navegador en tu teléfono
2. Navega a: **http://192.168.4.1**
3. La interfaz de control se cargará automáticamente

### Paso 4: Controlar el Carro

#### Joystick Izquierdo - Dirección (Steering)
- **Izquierda**: Gira a la izquierda
- **Derecha**: Gira a la derecha
- **Centro**: Dirección recta

#### Joystick Derecho - Velocidad (Throttle)
- **Arriba**: Avanza
- **Abajo**: Retrocede
- **Centro**: Detenido

#### Botón STOP
#### Botón STOP
- Presiona el botón rojo central para detener inmediatamente

### Paso 5: Ajustar Velocidad Máxima

Usa el slider "Max Speed" debajo de los controles para limitar la velocidad máxima del carro (10% a 100%). Ideal para interiores o para aprender a conducir.

## 🔧 Configuración

### Cambiar Credenciales WiFi

En `main.c`, modifica la configuración:

```c
web_control_config_t web_config = {
    .wifi_ssid = "MiCarroRC",           // Cambia el SSID
    .wifi_password = "mipassword123",    // Cambia la contraseña
    .wifi_channel = 1,
    .max_connections = 4,
    .server_port = 80,
    .motor_timeout_ms = 500              // Timeout de seguridad
};
```

### Ajustar Timeout de Seguridad

El timeout detiene los motores si no recibe comandos. Por defecto es 500ms.

```c
.motor_timeout_ms = 1000  // 1 segundo
```

## 📊 Indicadores de la Interfaz

### Barra Superior
- **WiFi Icon** - Estado de conexión
- **Latencia** - Tiempo de respuesta en ms (verde si <100ms)
- **Estado** - "Connected" o "Disconnected"

### Barra Inferior
- **Batería** - Porcentaje de batería (87%)
- **Velocidad** - Velocidad actual en km/h
- **Señal** - Intensidad de señal WiFi

## ⚠️ Solución de Problemas

### No puedo ver la red WiFi

1. Verifica que el ESP32 esté encendido
2. Revisa el monitor serial: debe decir "WiFi AP started"
3. Asegúrate de que el ESP32 esté cerca del teléfono

### La página no carga

1. Verifica que estés conectado a la red WiFi correcta
2. Usa exactamente: `http://192.168.4.1` (no https)
3. Intenta en modo incógnito del navegador
4. Limpia la caché del navegador

### Los motores no responden

1. Verifica las conexiones de los motores
2. Revisa el monitor serial para ver si llegan comandos
3. Presiona el botón STOP y vuelve a intentar
4. Verifica que la batería de los motores esté conectada

### Alta latencia (>200ms)

1. Acércate más al ESP32
2. Reduce interferencias WiFi (apaga otros dispositivos)
3. Cambia el canal WiFi en la configuración
4. Cierra otras apps en el teléfono

### El carro se detiene solo

Esto es normal - es el timeout de seguridad. Si no se reciben comandos por 500ms, los motores se detienen automáticamente por seguridad.

## 🔒 Seguridad

### Características de Seguridad Implementadas

1. **Timeout Automático** - Detiene motores si pierde conexión
2. **Validación de Comandos** - Limita valores entre -100 y +100
3. **Botón de Emergencia** - Detención inmediata
4. **Contraseña WiFi** - Protege el acceso

### Recomendaciones

- ⚠️ Siempre ten el botón STOP a mano
- ⚠️ Prueba primero en un área segura
- ⚠️ Mantén el carro a la vista
- ⚠️ No uses cerca de escaleras o agua

## 📡 Protocolo de Comunicación

### Comandos (HTTP POST a /command)

Formato binario:
```
Byte 0: Tipo de mensaje
  0x01 = Control de motor
  0x03 = Emergency stop

Para 0x01 (Control):
  Byte 1: Throttle (-100 a +100, signed)
  Byte 2: Steering (-100 a +100, signed)
  Byte 3: Checksum
```

### Telemetría (HTTP GET a /telemetry)

Respuesta JSON:
```json
{
  "battery": 87,
  "speed": 0.0,
  "signal": 100
}
```

## 🎨 Personalización de la Interfaz

Los archivos de la interfaz están en `components/web_control/www/`:

- **index.html** - Estructura HTML
- **style.css** - Estilos y diseño
- **app.js** - Lógica de control

Después de modificar, recompila:
```bash
idf.py build flash
```

## 📈 Rendimiento

- **Frecuencia de comandos**: 50Hz (cada 20ms)
- **Actualización de telemetría**: 10Hz (cada 100ms)
- **Latencia típica**: 80-120ms
- **Consumo WiFi**: ~50mA en AP mode

## 🔄 Próximas Mejoras

- [ ] Agregar video streaming (ESP32-CAM)
- [ ] Modo autónomo con sensores
- [ ] Grabación y replay de rutas
- [ ] Múltiples perfiles de velocidad
- [ ] Control por voz
- [ ] Telemetría avanzada (temperatura, corriente)

## 📞 Soporte

Si encuentras problemas:
1. Revisa el monitor serial para errores
2. Verifica las conexiones hardware
3. Asegúrate de usar ESP-IDF 5.5.1
4. Consulta la documentación técnica en `WEB_CONTROL_TECHNICAL.md`

---

**Versión**: 1.0  
**Compatibilidad**: ESP-IDF 5.5.1  
**Hardware**: ESP32-S3 + MX1508
