# Operation and APIs

[Back to project README](../README.md) | [Architecture](architecture.md) | [Hardware](hardware.md) | [Development](development.md)

## Control Modes

| Mode | Description |
| --- | --- |
| Auto | ESP32 uses the front ultrasonic distance and autonomous obstacle state machine. It stops, scans left/right with the servo, then turns toward clearance or briefly reverses. |
| Manual | Browser motor and servo controls. Drive buttons are hold-to-run; release sends Stop. A 900 ms command timeout stops motors if browser updates cease. |
| Audio | An external recognizer sends text commands over HTTP. Speech recognition does not run on the ESP32. Forward and turns use ultrasonic collision checks; reverse is limited to 750 ms because there is no rear sensor. |

Switching mode stops the motors, centers the servo, and resets the scan state. Choosing Auto resumes autonomous operation.

## Browser Control

1. Connect the laptop to the same router Wi-Fi as the ESP32.
2. Open the IP address printed by the ESP32 in Serial Monitor at 115200 baud.
3. Select Manual or Audio before using controls. Auto disables manual control widgets.
4. Hold a drive control to move; release it to send Stop. Use Stop for an immediate halt.

The web server has no authentication. Keep it on a trusted local Wi-Fi network and do not expose port 80 publicly.

## HTTP API

Requests use `application/x-www-form-urlencoded` data. Responses are JSON.

| Method and path | Form fields | Behavior |
| --- | --- | --- |
| `GET /` | None | Serves the browser control page. |
| `GET /api/status` | None | Returns `mode`, `movement`, `distanceCm`, and `message`. |
| `POST /api/mode` | `mode=auto`, `manual`, or `audio` | Stops motors, centers servo, and changes controller mode. |
| `POST /api/motor` | `command=forward`, `backward`, `left`, `right`, or `stop` | Manual/Audio drive. Forward and turns perform sonar checks; blocked or no-echo path returns HTTP 409. Backward warns that there is no rear sensor. |
| `POST /api/servo` | `direction=left`, `right`, or `straight` | Stops motors, positions servo, and returns `distanceCm` and `result`. |
| `POST /api/audio` | `text=<recognized phrase>` | Audio-mode text command; returns mode/status, movement result, or an error. |

### Audio Command Phrases

| Recognized text | Action |
| --- | --- |
| `start`, `go`, `forward`, `go forward` | Collision-checked forward |
| `stop`, `halt` | Stop motors |
| `backward`, `reverse`, `go backward` | Reverse for up to 750 ms; no rear sensor protection |
| `left`, `turn left` | Collision-checked left turn |
| `right`, `turn right` | Collision-checked right turn |
| `servo left`, `look left`, `sensor left` | Aim sensor left and return range |
| `servo right`, `look right`, `sensor right` | Aim sensor right and return range |
| `straight`, `look straight`, `servo straight` | Aim sensor forward and return range |
| `status`, `get status` | Return current state |
| `auto`, `autonomous`, `auto mode` | Return to autonomous mode |
| `manual`, `manual mode` | Switch to Manual mode |

Command text is trimmed, lowercased, and underscores/hyphens are normalized to spaces. Audio mode must be selected before calling `/api/audio`.

### Python Client Example

The speech recognizer supplies `recognized_text`; this example sends it to the ESP32 and reads the response.

```python
import requests

base = "http://192.168.1.42"  # Use the ESP32 IP printed to Serial Monitor
requests.post(f"{base}/api/mode", data={"mode": "audio"}, timeout=3)

recognized_text = "forward"  # Replace with speech-recognition output
response = requests.post(
    f"{base}/api/audio",
    data={"text": recognized_text},
    timeout=3,
)
print(response.status_code, response.json())
print(requests.get(f"{base}/api/status", timeout=3).json())
```

A blocked command returns HTTP 409 with a JSON `error`; a successful command returns HTTP 200. Stop audio-mode forward/turn movement by sending `stop` or changing mode.

## MQTT Telemetry

MQTT publishes to `esp32/sensors` through a dedicated FreeRTOS worker task. Event snapshots and periodic telemetry use the same 16-entry queue; broker TLS connection/reconnection and publishing are kept out of the navigation loop.

- Event names: `boot`, `movement`, `mode_changed`, `sensor_reading`, and `periodic`.
- Periodic telemetry interval: 5 seconds.
- Payload fields: `event`, `movement`, `servo`, `mode`, `distanceCm`, `temperatureC`, `humidityPercent`.
- When the queue is full, the oldest queued snapshot is dropped so the latest state can be retained.

---

*See [development and troubleshooting](development.md) for build and bring-up steps.*
