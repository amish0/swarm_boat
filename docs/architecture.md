# System Architecture

[Back to project README](../README.md) | [Hardware and wiring](hardware.md) | [Operation and APIs](operation.md) | [Development](development.md)

## High-Level System Diagram

```mermaid
flowchart LR
  subgraph Sensors[Inputs]
    US[Ultrasonic range sensor]
    DHT[DHT11 temperature and humidity]
  end

  subgraph ESP32[ESP32 WROOM firmware]
    Web[HTTP web server]
    Modes[Auto / Manual mode manager]
    Logic[Obstacle and command safety logic]
    Queue[16-entry MQTT event queue]
    Worker[FreeRTOS MQTT worker]
    Display[OLED and LED status]
  end

  Laptop[Browser client]
  Servo[Servo rotates range sensor]
  HBridge[L298N dual H-bridge]
  Motors[Left and right DC motors]
  OLED[SSD1306 128x64]
  LED[Four status LEDs]
  Broker[HiveMQ over TLS]
  Buzzer[Active buzzer and BC547 driver]

  Laptop -->|HTTP commands and status| Web
  Web --> Modes --> Logic
  US --> Logic
  Logic --> Servo --> US
  Logic --> HBridge --> Motors
  DHT --> Display --> OLED
  Logic --> Display --> LED
  Logic --> Buzzer
  Logic -->|event snapshots| Queue --> Worker -->|esp32/sensors| Broker
```

## Control Modes

| Mode | Controller | Behavior |
| --- | --- | --- |
| Auto | ESP32 | Samples the front range, stops for a detected obstacle, scans left/right, then turns toward available clearance or briefly reverses. |
| Manual | Browser | Hold-to-drive commands are sent over HTTP. Forward/left/right pass through ultrasonic clearance checks; manual drive has a command timeout. |

## Autonomous Decision Flow

```mermaid
flowchart TD
  Start([Control loop]) --> Samples[Take 3 front readings]
  Samples --> Median[Map timeout and calculate median]
  Median --> Close{Below 25 cm?}
  Close -->|Yes| Stop[Stop motors]
  Stop --> ScanLeft[Servo left; average 3 readings]
  ScanLeft --> ScanRight[Servo right; average 3 readings]
  ScanRight --> Choice{Either side at least 30 cm?}
  Choice -->|Yes| Turn[Counter-rotate toward clearer side for 350 ms]
  Choice -->|No| Reverse[Reverse for 250 ms]
  Turn --> StopAgain[Stop and resume front checks]
  Reverse --> StopAgain
  Close -->|No| Hysteresis{At least 35 cm?}
  Hysteresis -->|2 clear medians| Forward[Both motors forward]
  Hysteresis -->|25-34 cm| Hold[Keep current state]
  Hysteresis -->|Not yet confirmed| Samples
  Forward --> Samples
  Hold --> Samples
```

Front readings use a median-of-three filter. The 25 cm stop and 35 cm resume thresholds provide hysteresis. A `pulseIn()` timeout is represented as no echo; autonomous mode treats it as open range after confirmation. A disconnected sensor can therefore appear clear.

## MQTT Data Path

```mermaid
sequenceDiagram
  participant Control as Main control loop
  participant Queue as FreeRTOS event queue
  participant Worker as MQTT worker task
  participant Broker as HiveMQ
  Control->>Queue: Enqueue event snapshot (non-blocking)
  Control->>Queue: Enqueue periodic snapshot every 5 s
  Worker->>Broker: Connect / reconnect over TLS
  Worker->>Queue: Receive pending snapshot
  Worker->>Broker: Publish to esp32/sensors
```

The 16-entry queue carries boot, movement, mode, sensor-reading, and 5-second periodic events. If it fills while offline, the oldest queued event is discarded to make room for the latest state. Payload fields are documented in [Operation and APIs](operation.md).

## Safety Boundaries

- The ultrasonic sensor faces front/left/right only; reverse cannot be collision-checked.
- Manual reverse has no rear sensor protection.
- No-echo blocks manual forward. A manual turn can proceed on a side no-echo reading with a warning; Auto maps no-echo to max range.
- The browser server has no login and should remain on a trusted local Wi-Fi network.

---

*Architecture reference for the current firmware. See [hardware and wiring](hardware.md) before connecting power.*
