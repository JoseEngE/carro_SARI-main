# Guía de Hardware y Energía - RC Car

## 🔋 Configuración de Energía (SM5308 + Motores)

Al usar el módulo Power Bank **SM5308** para alimentar tanto el ESP32 como los motores desde la misma salida de 5V, debes tener mucho cuidado con el ruido eléctrico.

### Topología de Energía (Compartida)

```
[ Batería ] ── [ SM5308 Power Bank ] ──┬──► +5V ──► [ Driver Motores ] ──► [ Motores ]
                                       │
                                       └──► +5V ──► [ ESP32-S3 ] (¡RIESGO DE RUIDO!)
```

### ⚠️ Problema Crítico: Ruido y Brownouts
Los motores generan picos de voltaje que viajan por la línea de +5V y pueden reiniciar el ESP32.

**Solución OBLIGATORIA:**
1.  **Capacitor de Filtrado**: Conecta un capacitor electrolítico de **470µF a 1000µF (10V+)** directamente en los pines `5V` y `GND` del ESP32.
2.  **Cables**: Usa cables cortos y gruesos para la alimentación.

> [!NOTE]
> El módulo SM5308 puede apagarse automáticamente si el consumo es muy bajo (menos de ~50mA). Si el ESP32 se apaga solo cuando los motores están detenidos, es posible que el power bank esté entrando en modo de ahorro.

## ⚡ Brownout Prevention (ESP32 Reset Fix)

If your ESP32 resets when the steering motor is activated (especially if stalled), it is likely due to a voltage drop ("brownout").

### Solutions:
1.  **Add a Capacitor**: Place a large electrolytic capacitor (470µF to 1000µF, 10V or higher) across the power input pins (VIN/5V and GND) of the ESP32. This acts as a local energy reservoir.
2.  **Separate Power**: Use a dedicated 5V Buck Converter (Step-down) for the ESP32, separate from the motor power source, but sharing a common Ground.
3.  **Check Wiring**: Ensure wires from the battery to the motor driver are thick enough (at least 22AWG) to handle the surge current without significant voltage drop.
