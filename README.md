# Proyecto_Cubo_bot

Proyecto de robot educativo programable inspirado en la filosofía Bee-Bot, desarrollado con Arduino Nano. El robot utiliza tarjetas RFID MIFARE Classic para almacenar comandos de movimiento, permitiendo crear secuencias de navegación sin necesidad de programación tradicional.

El repositorio incluye código fuente para el robot, herramientas de grabación de tarjetas RFID, versiones con conectividad Bluetooth y archivos de fabricación PCB listos para producción.

---

## Características principales

* Programación mediante tarjetas RFID.
* Lectura de tarjetas MIFARE Classic con RC522.
* Ejecución de secuencias de movimiento almacenadas.
* Avance, retroceso y giros de 90°.
* Reinicio completo mediante tarjeta especial.
* Confirmación acústica mediante buzzer.
* Versión alternativa con comunicación Bluetooth.
* Archivos Gerber para fabricación de PCB.
* Compatible con Arduino Nano.

---

## Especificaciones técnicas

| Parámetro                    | Valor                      |
| ---------------------------- | -------------------------- |
| Microcontrolador             | Arduino Nano               |
| Alimentación                 | Según versión del proyecto |
| Lector RFID                  | MFRC522                    |
| Tipo de tarjetas             | MIFARE Classic             |
| Capacidad máxima de comandos | 30 (Modificable)           |
| Distancia por movimiento     | 16 cm                      |
| Pasos de avance              | 6250                       |
| Pasos de retroceso           | 5858                       |
| Pasos por centímetro         | 390.53                     |
| Giro de 90°                  | 3554 pasos                 |
| Ajuste de alineación         | 1000 pasos                 |
| Velocidad de paso            | 800 µs                     |
| Tipo de secuencia            | Half-Step (8 fases)        |
| Indicador sonoro             | Buzzer piezoeléctrico      |

---

## Tabla de comandos RFID

| Tarjeta | Acción                           |
| ------- | -------------------------------- |
| A       | Avanzar una celda                |
| B       | Retroceder una celda             |
| D       | Girar 90° a la derecha           |
| I       | Girar 90° a la izquierda         |
| V       | Ejecutar secuencia almacenada    |
| F       | Reiniciar completamente el robot |

---

## Precisión de movimiento

La versión actual está calibrada para desplazarse sobre tableros educativos con celdas de:

* 15 cm × 15 cm

Relación de movimiento aproximada:

* 390.53 pasos por centímetro
* 39.05 pasos por milímetro

Estas medidas pueden variar ligeramente dependiendo de:

* Diámetro de las ruedas.
* Estado de la batería.
* Tipo de superficie.
* Peso del robot.
* Tolerancias mecánicas del montaje.

---

## Hardware utilizado

### Electrónica

* Arduino Nano
* Módulo RFID RC522
* Buzzer piezoeléctrico
* Driver de motores
* Interruptor de encendido
* Batería recargable

### Mecánica

* Chasis impreso en 3D
* Ruedas motrices
* Rueda loca frontal
* Soportes para electrónica

---

## Arquitectura del sistema

RFID → Arduino Nano → Control de movimiento → Motores

RFID → Arduino Nano → Buzzer

Tarjetas RFID → Almacenamiento de comandos → Ejecución secuencial

---

## Estructura del repositorio

```text
Proyecto_Cubo_bot/
│
├── Codigo_Cubo_Bot_RFID/
│   ├── Versiones principales del robot
│
├── Codigo_Cubo_Bot_Bluetooth/
│   ├── Control mediante Bluetooth
│
├── Codigo_Grabar_Tarjetas/
│   ├── Herramientas para programar tarjetas RFID
│
├── PCB/
│   ├── Archivos Gerber
│   ├── FlatCAM
│   └── Diseño electrónico
│
├── Documentacion/
│   ├── Diagramas
│   ├── Manuales
│   └── Fotografías
│
└── README.md
```

---

## Funcionamiento

1. Acercar tarjetas RFID al lector RC522.
2. Cada tarjeta almacena un comando de movimiento.
3. El robot guarda los comandos en memoria.
4. La tarjeta "V" inicia la ejecución de la secuencia.
5. El robot realiza cada movimiento en orden.
6. Al finalizar emite una señal acústica de confirmación.

---

## Señales acústicas

| Evento             | Señal                |
| ------------------ | -------------------- |
| Lectura de tarjeta | Pitido corto         |
| Comando aceptado   | Doble pitido         |
| Error              | Triple pitido        |
| Fin de secuencia   | Doble pitido largo   |
| Reinicio           | Triple pitido rápido |

---

## Archivos PCB

El repositorio incluye:

* Archivos Gerber listos para fabricación.
* Archivos de mecanizado FlatCAM.
* Diseño de la placa utilizada por el robot.

---

## Aplicaciones educativas

* Pensamiento computacional.
* Programación por secuencias.
* Introducción a la robótica.
* Aprendizaje STEM.
* Lógica algorítmica.
* Resolución de problemas.

---

## Futuras mejoras

* Sensores de obstáculos.
* Comunicación WiFi.
* Aplicación móvil.
* Interfaz gráfica de programación.
* Seguimiento de líneas.
* Navegación autónoma.

---

## Autor

Ing. Mauricio Vasquez Menacho

Proyecto desarrollado con fines educativos y de aprendizaje en robótica, electrónica y programación embebida.

---

## Licencia

Este proyecto se distribuye bajo la licencia MIT. Se permite su uso, modificación y distribución conservando los créditos correspondientes al autor.
