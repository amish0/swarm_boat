# Development and Bring-Up

[Back to project README](../README.md) | [Architecture](architecture.md) | [Hardware](hardware.md) | [Operation](operation.md)

## Requirements

- PlatformIO Core or the PlatformIO VS Code extension.
- An ESP32 DevKit/WROOM development board supported as `esp32dev`.
- USB cable and serial monitor at 115200 baud.
- Required external libraries are listed in `platformio.ini`; Arduino core supplies `WebServer`, FreeRTOS, Wi-Fi, and OTA facilities.

## Build and Upload

From the project root:

```sh
pio run
pio run -t upload
pio device monitor -b 115200
```

For Arduino IDE, open the root `ai_boat.ino` entry point and select an ESP32 board and serial port. The maintained modules are in `src/` and shared declarations/configuration are in `include/`.

## First Bring-Up

1. Review [hardware and power wiring](hardware.md); do not use a TP4056 to charge a 2S series pack.
2. Lift the chassis so the wheels can spin freely before testing motor direction.
3. Confirm all grounds are common and that motor/servo supplies meet their voltage and current ratings.
4. Open Serial Monitor at 115200 baud and confirm the device joins the configured Wi-Fi; record its IP for the browser page.
5. Test Stop and each motor direction in Manual mode with the wheels lifted.
6. Check that servo Left/Right/Straight correspond to the physical sensor direction.
7. Test the range sensor with known distances, including timeout/no-echo behavior.
8. Test MQTT connectivity and inspect `esp32/sensors` for boot, movement, mode, sensor, and periodic events.
9. Test obstacle stops and mode transitions in a clear, controlled area before placing the boat in water.

## Configuration Map

- GPIO assignments, thresholds, buzzer timings, and the MQTT interval: `include/config.h`.
- Movement/servo enumerations and obstacle states: `include/constants.h`, `include/main.h`.
- Wi-Fi and MQTT credentials: currently initialized in `include/main.h`; move these into private local configuration before sharing or deploying.
- Libraries and target board: `platformio.ini`.

## Troubleshooting

| Symptom | Checks |
| --- | --- |
| Browser page unreachable | Put laptop and ESP32 on the same Wi-Fi; use the current IP printed in Serial Monitor; port 80 is local HTTP. |
| MQTT events not arriving | Confirm Wi-Fi, HiveMQ host/port 8883, credentials, broker TLS configuration, and MQTT worker startup messages. |
| No sonar reading | Check TRIG GPIO 5, ECHO GPIO 18, common ground, sensor supply, and ECHO level shifting; `NO ECHO` can also mean target/range/alignment issues. |
| ESP32 resets when servo moves | Power servo from a suitable regulated supply, share grounds, and avoid loading the HW-131 beyond its rating. |
| One motor direction is reversed | Verify L298N input/output channel mapping and motor lead orientation; test with the chassis lifted. |

## Release Notes

- [v1.0.0](../releases/v1.0.0.md)
- [v2.0.0](../releases/v2.0.0.md)
- [v3.0.0](../releases/v3.0.0.md)

---

*For a release, verify the version metadata, build the tagged firmware, and test on the target hardware.*
