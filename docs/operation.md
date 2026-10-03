# Operation and APIs

[Back to project README](../README.md) | [Architecture](architecture.md) | [Hardware](hardware.md) | [Development](development.md)

## Control Modes

| Mode | Description |
| --- | --- |
| Auto | ESP32 uses the front ultrasonic distance and autonomous obstacle state machine. It stops, scans left/right with the servo, then turns toward clearance or briefly reverses. |
| Manual | Browser motor and servo controls. Drive buttons are hold-to-run; release sends Stop. A 900 ms command timeout stops motors if browser updates cease. |

Switching mode stops the motors, centers the servo, and resets the scan state. Choosing Auto resumes autonomous operation.

## Browser Control

1. Connect the laptop to the same router Wi-Fi as the ESP32.
2. Open the IP address printed by the ESP32 in Serial Monitor at 115200 baud.
3. Select Manual before using the drive controls. Auto disables manual control widgets.
4. Hold a drive control to move; release it to send Stop. Use Stop for an immediate halt.

The web server has no authentication. Keep it on a trusted local Wi-Fi network and do not expose port 80 publicly.

## HTTP API

Requests use `application/x-www-form-urlencoded` data. Responses are JSON.

| Method and path | Form fields | Behavior |
| --- | --- | --- |
| `GET /` | None | Serves the browser control page. |
| `GET /api/status` | None | Returns `mode`, `movement`, `distanceCm`, and `message`. |
| `POST /api/mode` | `mode=auto` or `manual` | Stops motors, centers servo, and changes controller mode. |
| `POST /api/motor` | `command=forward`, `backward`, `left`, `right`, or `stop` | Manual drive. Forward requires a valid front sonar return and clear path. Turns are blocked by a measured close obstacle; a side no-echo scan permits the requested turn with a warning. Backward warns that there is no rear sensor. |
| `POST /api/servo` | `direction=left`, `right`, or `straight` | Stops motors, positions servo, and returns `distanceCm` and `result`. |

## MQTT Telemetry

MQTT publishes to `esp32/sensors` through a dedicated FreeRTOS worker task. Event snapshots and periodic telemetry use the same 16-entry queue; broker TLS connection/reconnection and publishing are kept out of the navigation loop.

- Event names: `boot`, `movement`, `mode_changed`, `sensor_reading`, and `periodic`.
- Periodic telemetry interval: 5 seconds.
- Payload fields: `event`, `movement`, `servo`, `mode`, `distanceCm`, `temperatureC`, `humidityPercent`.
- When the queue is full, the oldest queued snapshot is dropped so the latest state can be retained.

---

*See [development and troubleshooting](development.md) for build and bring-up steps.*
