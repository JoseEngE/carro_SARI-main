# Wall Follower Component

Este componente implementa el **Modo Autónomo** del vehículo, permitiéndole navegar sin intervención humana evadiendo obstáculos y manteniéndose centrado entre paredes utilizando tres sensores de distancia (frontal, izquierdo y derecho).

## Funcionamiento del Algoritmo Autónomo

El algoritmo toma decisiones en cada iteración de control (a través de la función `wall_follower_process`) basándose en las lecturas de los sensores de distancia. La toma de decisiones se rige por las siguientes reglas lógicas:

### 1. Evasión de Colisiones Frontales (Máxima Prioridad)
La seguridad frontal es crítica. El sistema evalúa constantemente la distancia medida por el sensor frontal (`d_front`).
- Si `d_front` es **menor** a la distancia de seguridad configurada (`min_front_dist_mm`, ej. 350 mm), el vehículo detecta un obstáculo inminente frente a él.
- Al detectar el obstáculo, el algoritmo compara la distancia abierta a su izquierda (`d_left`) y a su derecha (`d_right`) para determinar qué lado presenta la mejor vía de escape.
- Ejecuta un **giro brusco** (ángulo al tope, -100 hacia la izquierda o +100 hacia la derecha) hacia el lado más despejado. Esto se realiza rodando a una velocidad de giro específica (`turn_speed`).

### 2. Centrado de Carril / Seguimiento de Paredes (Navegación Normal)
Si el camino al frente está despejado (no hay obstáculos críticos), el vehículo avanza e intenta mantenerse centrado entre los posibles muros laterales.
- Procede hacia adelante a la velocidad crucero dictada por `base_speed`.
- Cuantifica el error de posición calculando la diferencia entre los sensores laterales: `Diferencia = d_left - d_right`.
- **Control Proporcional (P):** Utiliza un controlador proporcional para calcular la corrección de la dirección. Multiplica la diferencia por la constante `kp_steering`.
  - Si `d_left > d_right` (el vehículo está más cerca de la pared derecha), el sistema computa un ángulo negativo para forzar al servo a virar hacia la izquierda, volviendo al centro.
  - Si `d_left < d_right` (más cerca de la pared izquierda), produce un ángulo positivo para corregir la trayectoria hacia la derecha.
- El ángulo resultante generado por el controlador es limitado automáticamente al rango válido de dirección de servo, que va de `[-100, 100]`.

### Filtrado de Lecturas Inválidas
Para garantizar estabilidad, se depuran las lecturas fallidas:
- Los sensores ToF suelen reportar valores atípicamente altos (como `> 8000 mm`) cuando el objetivo está fuera de rango. El algoritmo descarta e intercepta estas anomalías para limitar las lecturas laterales al umbral de `side_wall_max_dist_mm` y la frontal a 2 metros seguros, garantizando que el cálculo del error proporcional nunca dispare ángulos incontrolables.

## Configuración y Parámetros (`wall_follower_config_t`)

El desempeño en pista puede ser calibrado mediante la estructura de configuración:

| Parámetro | Descripción | Valor Típico |
| :--- | :--- | :--- |
| `min_front_dist_mm` | Distancia frontal mínima para gatillar giros de emergencia y evitar choque. | `350` mm |
| `side_wall_max_dist_mm` | Límite de "visión lateral" para evitar que pasillos muy anchos produzcan lecturas irreales y alteren la ganancia. | `500` mm |
| `base_speed` | Potencia (PWM) transmitida a los motores durante el recorrido libre. | `70` % |
| `turn_speed` | Potencia de los motores aplicada durante la evasión esquiva. | `60` % |
| `kp_steering` | Constante del Controlador Proporcional para la dirección. Un valor superior hace las enmiendas más bruscas y rápidas. | `1.0f` |

## Ciclo de Ejecución (API)

El sistema para invocar este modo requeriría típicamente esta lógica:
1. Conseguir e inicializar los parámetros: `wall_follower_init(&config)` cargando la configuración base (`wall_follower_get_default_config()`).
2. Adquirir en un bucle periódico las dimensiones reales de los 3 sensores.
3. Evaluar el siguiente movimiento derivando el estado a **`wall_follower_process()`**, que entregará como referencia unos valores apuntados correspondientes al **speed** y al **steering**.
4. Inyectar inmediatamente este par de valores en el componente de motores y actuadores de control del servo.
