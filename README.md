# ESP32 Obstacle-Avoiding Boat

Firmware for an ESP32 boat using an L298N dual H-bridge, two DC motors, a servo-mounted ultrasonic sensor, an OLED, and a DHT11. The controller samples the front range, scans both sides when an obstacle is close, and chooses a short turn or reverse action.

## System Block Diagram

```mermaid
flowchart LR
  subgraph Inputs
    US[Ultrasonic sensor\nfront / left / right]
    DHT[DHT11\ntemperature / humidity]
  end

  subgraph Controller[ESP32 firmware]
    Range[Range filtering]
    Avoid[Obstacle state machine]
    View[OLED / LED status]
    Network[Wi-Fi / OTA / HTTP control]
  end

  Servo[Servo\nrotates ultrasonic sensor]
  Driver[L298N dual H-bridge]
  Motors[Left and right DC motors]
  OLED[SSD1306 OLED]
  LEDs[Status LEDs]

  US --> Range --> Avoid
  Avoid --> Servo --> US
  Avoid --> Driver --> Motors
  DHT --> View --> OLED
  Avoid --> View
  Network -. OTA and browser commands .-> Controller
  View --> LEDs
```

## Wiring Map

| Device signal | ESP32 GPIO | Connection |
| --- | ---: | --- |
| L298N IN1 | 25 | Left motor direction input 1 |
| L298N IN2 | 26 | Left motor direction input 2 |
| L298N IN3 | 27 | Right motor direction input 1 |
| L298N IN4 | 32 | Right motor direction input 2 |
| Buzzer driver control | 14 | Drives BC547 base through a 1 kOhm resistor |
| Ultrasonic TRIG | 5 | Trigger input |
| Ultrasonic ECHO | 18 | Echo output, through a 5 V-to-3.3 V level divider when required |
| Servo signal | 13 | Sensor scanning servo |
| DHT11 data | 33 | Temperature/humidity data |
| OLED SDA | 21 | I2C data |
| OLED SCL | 22 | I2C clock |
| Stop LED | 15 | Red status LED |
| Forward/turn LED | 4 | Green status LED |
| Reverse LED | 23 | Blue status LED |
| Heartbeat LED | 19 | Green heartbeat |
| OTA LED | 2 | ESP32 onboard LED |

```mermaid
flowchart LR
  ESP[ESP32] -->|GPIO 25-26: IN1-IN2| L298N[L298N]
  ESP -->|GPIO 27, 32: IN3-IN4| L298N
  L298N -->|OUT1-OUT2| LM[Left motor]
  L298N -->|OUT3-OUT4| RM[Right motor]
  ESP -->|GPIO 5 TRIG / GPIO 18 ECHO| US[Ultrasonic sensor]
  ESP -->|GPIO 13 signal| SV[Servo]
  SV -. rotates .-> US
  ESP -->|GPIO 33| DHT[DHT11]
  ESP -->|GPIO 21 SDA / GPIO 22 SCL| OLED[SSD1306 OLED]
```

The firmware assumes the L298N `ENA` and `ENB` jumpers are installed, so both channels run at full speed. Use a suitable motor supply and servo supply; connect their grounds to ESP32 ground. Do not power motors from the ESP32 3.3 V pin. Confirm the ultrasonic ECHO voltage is safe for the ESP32 before connecting it.

## Power and Full Wiring Schematic

**Do not connect the series pair of 3.7 V cells to a TP4056.** The two series cells form a 2S lithium-ion pack (7.4 V nominal, 8.4 V fully charged), while a TP4056 is for one cell (1S). For this series pack, use a 2S BMS with balancing and a compatible 8.4 V CC/CV 2S charger. The BMS is not itself a charger.

```mermaid
flowchart TB
  C1["Li-ion cell 1<br/>3.7 V nominal"] --> SERIES["Two cells in series<br/>2S: 7.4 V nominal, 8.4 V full"]
  C2["Li-ion cell 2<br/>3.7 V nominal"] --> SERIES
  SERIES --> BMS["2S BMS with balancing"]
  CHARGER["8.4 V CC/CV 2S charger"] -->|BMS charge terminals per board labels| BMS
  TP["TP4056 1S charger<br/>DO NOT connect to 2S pack"] -. incompatible .-> SERIES
  BMS -->|Protected pack output| PACK["Protected battery rail<br/>P+ / P-"]

  PACK -->|Set to motor rated voltage| MOTORREG["Motor supply regulator<br/>only if required by motor rating"]
  MOTORREG --> L298N["L298N motor supply VS"]
  L298N -->|OUT1 / OUT2| LEFT["Left DC motor"]
  L298N -->|OUT3 / OUT4| RIGHT["Right DC motor"]

  PACK --> HW131["HW-131 breadboard power module<br/>input from protected 2S pack"]
  HW131 -->|5 V rail| VIN["ESP32 DevKit 5V / VIN<br/>bare WROOM needs 3.3 V regulator"]
  HW131 -->|5 V rail if current capacity allows| SERVO5["Servo +5 V"]
  HW131 -->|5 V rail, if buzzer is rated 5 V| BUZZ5["Buzzer +"]
  VIN --> ESP[ESP32]
  ESP -->|GPIO 25, 26, 27, 32| L298N
  ESP -->|GPIO 13 signal| SERVO[Servo]
  SERVO5 --> SERVO
  ESP -->|GPIO 14 through 1 kOhm| BASE["BC547 base"]
  BASE --> BUZZER["BC547 switches buzzer negative"]
  BUZZ5 --> BUZZER

  ESP -->|GPIO 5 TRIG| US["Ultrasonic sensor"]
  US -->|ECHO through divider if 5 V| ESP
  ESP -->|GPIO 33| DHT[DHT11]
  ESP -->|GPIO 21 SDA / GPIO 22 SCL| OLED[SSD1306 OLED]
  ESP -->|GPIO 15 / 4 / 23 / 19| LEDS["4 LEDs, each with its own series resistor"]

  GND[Common ground] --- BMS
  GND --- HW131
  GND --- L298N
  GND --- ESP
  GND --- SERVO
  GND --- US
  GND --- DHT
  GND --- OLED
  GND --- BUZZER
```

The HW-131 shown is the common breadboard supply module with selectable 5 V/3.3 V rails; verify the labels and input range on your exact board. A 2S pack ranges from about 6.0 V to 8.4 V, while many HW-131/MB102 versions specify a 6.5 V minimum input. Connect the pack directly only if your module accepts the entire pack voltage range; otherwise use a suitable input DC/DC converter. Select the 5 V rail for an ESP32 DevKit `5V/VIN` input and the servo only if the HW-131 can supply the servo's peak current. Its small linear regulators can overheat or cause ESP32 resets under servo load; use a separate adequately rated 5 V regulator for the servo if needed, with grounds connected. Never feed 5 V to a bare ESP32-WROOM module.

The listed `3.7 V charging module` is not identified beyond its voltage. If it is a TP4056, leave it disconnected from this 2S series pack. Do not charge the pack until the charger and BMS are confirmed as 2S-compatible. Check the motors' rated voltage before feeding the pack to L298N `VS`; the pack reaches 8.4 V when full. Many L298N boards also have a `5V-EN` regulator jumper: follow that module's markings and avoid connecting two 5 V regulator outputs together.

The diagram assumes an ESP32 DevKit board. A bare ESP32-WROOM module cannot take 5 V on its supply pin and requires a regulated 3.3 V supply. The servo should use a regulated 5 V supply capable of its peak current, with grounds shared; do not power it from the ESP32 3.3 V output.

For the four LEDs, connect GPIO 15 (red), GPIO 4 (green), GPIO 23 (blue), and GPIO 19 (heartbeat) through four separate 220-330 Ohm resistors to LED anodes; connect each cathode to ground. For a bare DHT11, add a 4.7-10 kOhm pull-up from data to 3.3 V; many DHT11 modules already include it.

### Two-Pin Buzzer Wiring

Use the BC547 NPN transistor as a low-side switch. Do not drive a buzzer directly from an ESP32 GPIO.

```mermaid
flowchart LR
  GPIO[ESP32 GPIO 14] --> R[1 kOhm resistor]
  R --> B[BC547 base]
  E[BC547 emitter] --> GND[Common ground]
  C[BC547 collector] --> BUZZMIN[Buzzer minus]
  SUPPLY[Rated buzzer supply] --> BUZZPLUS[Buzzer plus]
  BUZZPLUS --> BUZZER[Two-pin active buzzer]
  BUZZER --> BUZZMIN
  GND --- ESPGND[ESP32 GND]
  GND --- SUPPLYGND[Buzzer supply ground]
```

Connect the buzzer's `+` pin to its rated supply, its `-` pin to the transistor collector, the emitter to ground, and GPIO 14 to the base through the resistor. Join the buzzer supply ground and ESP32 ground. Verify the buzzer voltage and current from its datasheet. For a magnetic/inductive buzzer, add a flyback diode across the buzzer (cathode to `+`).

The current firmware toggles GPIO 14 on and off and is intended for an **active buzzer**, which generates its own audio tone. A passive piezo needs a frequency-driven tone signal; this on/off pattern will not produce the intended continuous beep on a passive part.

## Obstacle Logic

The front distance is sampled three times and filtered with the median. A `pulseIn()` timeout is represented as no echo; it is treated as open range for navigation. The OLED reports `NO ECHO` when all three front samples time out.

```mermaid
flowchart TD
  Start([Start loop]) --> Sample[Collect 3 front readings]
  Sample --> Filter[Map timeouts to max range and take median]
  Filter --> Near{Median below 25 cm?}
  Near -->|Yes| Stop[Stop both motors]
  Stop --> PointLeft[Point servo left]
  PointLeft --> SettleLeft[Wait 300 ms]
  SettleLeft --> ReadLeft[Average 3 left readings]
  ReadLeft --> PointRight[Point servo right]
  PointRight --> SettleRight[Wait 300 ms]
  SettleRight --> ReadRight[Average 3 right readings]
  ReadRight --> Center[Center servo]
  Center --> ClearSide{Either side at least 30 cm?}
  ClearSide -->|Yes| Choose[Turn toward side with greater clearance]
  Choose --> Turn[Counter-rotate motors for 350 ms]
  Turn --> Halt[Stop, then sample front again]
  ClearSide -->|No| Reverse[Reverse both motors for 250 ms]
  Reverse --> Halt
  Near -->|No| Clear{Median at least 35 cm?}
  Clear -->|Yes| ClearCount{Two clear medians?}
  ClearCount -->|No| Sample
  ClearCount -->|Yes| Forward[Drive both motors forward]
  Forward --> Sample
  Clear -->|No: 25-34 cm| Hold[Keep current motor state]
  Hold --> Sample
```

The 25 cm stop and 35 cm resume thresholds provide hysteresis. The median rejects isolated spikes; each side uses three readings averaged together. A timeout is treated as open range, so a disconnected or miswired sensor can also appear clear. Test with the propellers clear of people and objects.

## Movement State Diagram

```mermaid
stateDiagram-v2
  [*] --> Stop
  Stop --> Forward: two clear medians >= 35 cm
  Forward --> Stop: front median < 25 cm
  Stop --> ScanLeft: confirmed front obstacle
  Forward --> ScanLeft: confirmed front obstacle
  state ScanLeft {
    [*] --> ServoLeft
    ServoLeft --> MeasureLeft: settle and average 3 samples
    MeasureLeft --> ServoRight
    ServoRight --> MeasureRight: settle and average 3 samples
    MeasureRight --> [*]: center servo
  }
  ScanLeft --> TurnLeft: left has greater clearance
  ScanLeft --> TurnRight: right has greater clearance
  ScanLeft --> Reverse: both sides below 30 cm
  TurnLeft --> Stop: after 350 ms
  TurnRight --> Stop: after 350 ms
  Reverse --> Stop: after 250 ms
  Stop --> Stop: median 25-34 cm
```

| State | Left motor | Right motor |
| --- | --- | --- |
| Forward | Forward | Forward |
| Backward | Backward | Backward |
| Left | Backward | Forward |
| Right | Forward | Backward |
| Stop | Stopped | Stopped |

## Runtime and Indicators

- The servo starts centered, then scans left and right only after a close front obstacle is detected.
- The OLED shows movement state, filtered front distance, temperature, and humidity.
- Red indicates stop, green indicates forward or turning, and blue indicates reverse. The heartbeat LED blinks once per second.
- The buzzer gives short repeating beeps during side scans and backward/turning movement; it is silent while stopped or moving forward.
- Wi-Fi and password-protected ArduinoOTA are enabled. OTA handling runs in the main loop.
- A browser control page is served over HTTP on port 80 after the ESP32 joins the configured router Wi-Fi. Connect the laptop to the same network and open the URL printed to Serial Monitor at `115200` baud.
- Select `Manual` to enable the drive and servo controls. Hold a direction button to drive; releasing it sends Stop. A 900 ms command timeout stops the motors if updates from the browser stop arriving.
- Manual Forward checks the straight-ahead range; Left and Right aim the sonar in that direction before checking. Each check uses three samples. An obstacle inside the configured clearance or any no-echo sample blocks the command and displays the reason. While held, direction commands are safety-checked again periodically.
- Manual Backward remains available but warns that there is no rear-facing sensor, so reverse is not collision-protected. Audio Backward is limited to a 750 ms pulse for the same reason.
- The third mode, `Audio`, accepts recognized text from an external speech-recognition client; recognition and microphone access do not run on the ESP32.
- In Audio mode, send URL-encoded form data to `POST /api/audio` with a `text` field. Accepted text includes `start`/`forward`, `stop`, `backward`, `left`, `right`, `servo left`, `servo right`, `straight`, and `status`. `start` means collision-checked forward movement. `auto` or `autonomous` returns to autonomous mode; `manual` switches to Manual mode.
- Motor movement commands use the same ultrasonic collision checks as Manual mode. A blocked command returns HTTP 409 with a JSON `error`; success returns JSON. Reverse remains unprotected because there is no rear sensor.
- `GET /api/status` returns the current mode, movement, and front distance as JSON. Mode can also be selected with `POST /api/mode` using `mode=auto`, `mode=manual`, or `mode=audio`.
- To test without speech recognition, select Audio in the control page and type a recognized phrase into the text field. A Python client using `requests` can follow this pattern:

```python
import requests

base = "http://192.168.1.42"  # replace with the ESP32 IP printed to Serial
requests.post(f"{base}/api/mode", data={"mode": "audio"}, timeout=3)

recognized_text = "forward"  # replace with the speech recognizer result
response = requests.post(
  f"{base}/api/audio",
  data={"text": recognized_text},
  timeout=3,
)
print(response.status_code, response.json())
print(requests.get(f"{base}/api/status", timeout=3).json())
```
- Manual servo `Left`, `Right`, and `Straight` requests stop the motors, point the sensor, wait for it to settle, then return a three-sample distance with `object`, `clear`, or `no_echo` status.
- Selecting `Auto` stops the motors, centers the servo, resets the scan state, and returns control to autonomous obstacle avoidance.
- The HTTP control page has no login. Use it only on a trusted local Wi-Fi network; do not expose port 80 to the public internet.
- MQTT events publish on `esp32/sensors` from a dedicated FreeRTOS worker task. The control loop only copies a small event snapshot into a 16-entry queue; TLS connect/reconnect and publishing happen in the worker, so broker delays do not block motor or sensor logic. Events include `boot`, `movement`, `mode_changed`, and `sensor_reading`, plus a `periodic` telemetry event every 5 seconds. Each JSON payload includes event name, movement, servo position, mode, `distanceCm`, `temperatureC`, and `humidityPercent`. If the queue fills while offline, the oldest snapshot is dropped to retain the latest state.
- Wi-Fi credentials are currently held in the firmware source. Move them to a private local configuration before sharing the project.

## Build and Upload

The project targets the PlatformIO `esp32dev` board with the Arduino framework:

```sh
pio run
```

Open `ai_boat.ino` for the Arduino IDE entry point. The application implementation is in `src/` and shared interfaces/configuration are in `include/`.

## Project Layout

```text
src/
  main.cpp                 Navigation and motor-driver control
  sensorData.cpp           Ultrasonic and DHT11 readings
  servoController.cpp      Sensor scanning servo
  displayController.cpp    SSD1306 status display
  ledcontroller.cpp        Movement and heartbeat LEDs
  ota.cpp                  Wi-Fi OTA setup and handling
include/
  config.h                 GPIO assignments and range thresholds
  constants.h              Movement and servo states
  main.h                   Application state
platformio.ini             ESP32 build configuration
ai_boat.ino                Arduino IDE entry point
```