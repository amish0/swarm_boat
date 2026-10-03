#include "../include/main.h"
#include <stdio.h>
#include <string.h>

static long medianOfThree(long first, long second, long third)
{
  long temporary;
  if (first > second) {
    temporary = first;
    first = second;
    second = temporary;
  }
  if (second > third) {
    temporary = second;
    second = third;
    third = temporary;
  }
  if (first > second) {
    temporary = first;
    first = second;
    second = temporary;
  }
  return second;
}

static const char CONTROL_PAGE[] PROGMEM = R"rawliteral(
<!doctype html>
<html lang="en">
<head>
<meta charset="utf-8">
<meta name="viewport" content="width=device-width,initial-scale=1">
<title>Boat Control</title>
<style>
:root{color-scheme:light;--ink:#1f2923;--muted:#647168;--line:#d2d9cf;--paper:#edf1e9;--surface:#fbfcf7;--green:#176b50;--green-dark:#104d3b;--amber:#e6a23b;--red:#b33e35;--shadow:0 14px 34px rgba(35,54,42,.09)}
*{box-sizing:border-box}body{min-height:100vh;margin:0;background-color:var(--paper);background-image:radial-gradient(ellipse at 8% 0%,rgba(201,222,199,.62),transparent 38%),linear-gradient(135deg,transparent 49.8%,rgba(255,255,255,.3) 50%,transparent 50.2%);color:var(--ink);font:16px/1.45 "Avenir Next","Trebuchet MS",sans-serif}
main{width:min(100% - 32px,680px);margin:0 auto;padding:26px 0 34px;animation:appear .35s ease-out}@keyframes appear{from{opacity:0;transform:translateY(8px)}to{opacity:1;transform:translateY(0)}}
header{display:flex;justify-content:space-between;align-items:center;padding:2px 2px 19px;border-bottom:1px solid var(--line)}h1{font-size:28px;line-height:1.1;margin:4px 0 0;font-weight:750}.eyebrow{font:700 11px/1.2 "Avenir Next","Trebuchet MS",sans-serif;color:var(--green);text-transform:uppercase}
.connection{display:inline-flex;align-items:center;gap:8px;padding:7px 10px;border:1px solid var(--line);background:rgba(251,252,247,.7);color:var(--muted);font-size:13px;font-weight:700}.connection::before{content:"";width:8px;height:8px;border-radius:50%;background:var(--amber)}.connection.online{color:var(--green);border-color:#a9c8b4}.connection.online::before{background:#339565}.connection.offline{color:var(--red);border-color:#dfb3ad}.connection.offline::before{background:var(--red)}
.readout{display:grid;grid-template-columns:repeat(3,minmax(0,1fr));gap:0;margin:18px 0 28px;background:var(--surface);border:1px solid var(--line);box-shadow:var(--shadow)}.metric{min-width:0;padding:14px 16px}.metric+.metric{border-left:1px solid var(--line)}.label{font-size:11px;font-weight:700;text-transform:uppercase;color:var(--muted)}.value{margin-top:5px;font:700 20px/1.15 "Avenir Next","Trebuchet MS",sans-serif;overflow-wrap:anywhere}
section{margin:25px 0}section h2{font-size:13px;text-transform:uppercase;color:var(--muted);margin:0 0 10px;font-weight:750}.modes{display:grid;grid-template-columns:repeat(2,1fr);max-width:420px;padding:4px;background:#dfe6dc;border:1px solid #c8d2c8}.modes button{border:0;background:transparent;min-height:43px}.modes button.active{background:var(--green);color:#fff;box-shadow:0 3px 9px rgba(23,107,80,.2)}
button{min-height:52px;border:1px solid var(--line);background:var(--surface);color:var(--ink);font:700 15px/1.2 "Avenir Next","Trebuchet MS",sans-serif;padding:10px 14px;touch-action:none;cursor:pointer;transition:background-color .12s,border-color .12s,color .12s,transform .12s}button:active{transform:translateY(1px)}button:focus-visible{outline:3px solid var(--amber);outline-offset:2px}button:disabled{opacity:.4;cursor:not-allowed}
.drive{display:grid;grid-template-columns:repeat(3,minmax(0,1fr));gap:9px;max-width:400px;margin:0 auto;padding:14px;background:rgba(251,252,247,.55);border:1px solid var(--line)}.drive button{min-height:72px;display:flex;flex-direction:column;align-items:center;justify-content:center;gap:3px}.drive .up{grid-column:2}.drive .left{grid-column:1;grid-row:2}.drive .stop{grid-column:2;grid-row:2;color:var(--red);border-color:#e1beb7}.drive .right{grid-column:3;grid-row:2}.drive .down{grid-column:2;grid-row:3}.drive [data-drive]:active{background:var(--green);border-color:var(--green);color:#fff}.glyph{font-size:23px;line-height:1}
.sensors{display:flex;gap:8px}.sensors button{flex:1;min-height:46px}.sensors button.active{background:var(--green);border-color:var(--green);color:#fff}#message{min-height:28px;margin:12px 0 0;padding:9px 11px;border-left:3px solid var(--amber);background:rgba(251,252,247,.72);color:var(--muted);font-size:14px}
@media(max-width:520px){main{width:min(100% - 22px,680px);padding-top:18px}.metric{padding:11px 9px}.value{font-size:17px}.drive{gap:7px;padding:10px}.drive button{min-height:66px}.sensors{gap:6px}.sensors button{padding:8px 5px;font-size:14px}}
@media(prefers-reduced-motion:reduce){*,*::before,*::after{animation-duration:.01ms!important;transition-duration:.01ms!important}}
</style>
</head>
<body><main>
<header><div><div class="eyebrow">ESP32 / Field Console</div><h1>Boat Control</h1></div><span id="connection" class="connection"><span id="connectionLabel">Connecting</span></span></header>
<div class="readout"><div class="metric"><div class="label">Mode</div><div class="value" id="mode">--</div></div><div class="metric"><div class="label">Movement</div><div class="value" id="movement">--</div></div><div class="metric"><div class="label">Front range</div><div class="value" id="range">--</div></div></div>
<section><h2>Control mode</h2><div class="modes" role="group" aria-label="Control mode"><button id="auto" aria-pressed="true">Auto</button><button id="manual" aria-pressed="false">Manual</button></div></section>
<section><h2>Drive</h2><div class="drive">
<button class="up" data-drive="forward" aria-label="Drive forward"><span class="glyph" aria-hidden="true">&#8593;</span><span>Forward</span></button><button class="left" data-drive="left" aria-label="Turn left"><span class="glyph" aria-hidden="true">&#8592;</span><span>Left</span></button><button class="stop" id="stop"><span class="glyph" aria-hidden="true">&#9632;</span><span>Stop</span></button><button class="right" data-drive="right" aria-label="Turn right"><span class="glyph" aria-hidden="true">&#8594;</span><span>Right</span></button><button class="down" data-drive="backward" aria-label="Drive backward"><span class="glyph" aria-hidden="true">&#8595;</span><span>Backward</span></button>
</div></section>
<section><h2>Sensor direction</h2><div class="sensors"><button data-servo="left">&#8592; Left</button><button data-servo="straight">Straight</button><button data-servo="right">Right &#8594;</button></div></section>
<p id="message" role="status"></p>
</main>
<script>
const $=s=>document.querySelector(s),post=(url,data)=>fetch(url,{method:'POST',headers:{'Content-Type':'application/x-www-form-urlencoded'},body:new URLSearchParams(data)}).then(async r=>{let j=await r.json();if(!r.ok)throw Error(j.error||'Request failed');return j});
let mode='auto',driveTimer=null,driveSending=false,stopQueued=false;
async function refresh(){try{const r=await fetch('/api/status'),s=await r.json();mode=s.mode;$('#mode').textContent=mode.toUpperCase();$('#movement').textContent=s.movement;$('#range').textContent=s.distanceCm<0?'NO ECHO':`${s.distanceCm} cm`;$('#connectionLabel').textContent='Online';$('#connection').classList.add('online');$('#connection').classList.remove('offline');['auto','manual'].forEach(m=>{$('#'+m).classList.toggle('active',mode===m);$('#'+m).setAttribute('aria-pressed',String(mode===m))});document.querySelectorAll('[data-drive],[data-servo],#stop').forEach(b=>b.disabled=mode==='auto');if(s.message)$('#message').textContent=s.message}catch(e){$('#connectionLabel').textContent='Offline';$('#connection').classList.add('offline');$('#connection').classList.remove('online')}}
$('#auto').onclick=async()=>{try{await post('/api/mode',{mode:'auto'});$('#message').textContent='Autonomous control enabled'}catch(e){$('#message').textContent=e.message}refresh()};
$('#manual').onclick=async()=>{try{await post('/api/mode',{mode:'manual'});$('#message').textContent='Manual control enabled'}catch(e){$('#message').textContent=e.message}refresh()};
async function drive(command){if(driveSending){if(command==='stop')stopQueued=true;return}driveSending=true;try{const r=await post('/api/motor',{command});if(r.warning)$('#message').textContent=r.warning;else if(command==='stop')$('#message').textContent='Stopped'}catch(e){$('#message').textContent=e.message}finally{driveSending=false;if(stopQueued){stopQueued=false;drive('stop')}}}
document.querySelectorAll('[data-drive]').forEach(b=>{b.onpointerdown=e=>{e.preventDefault();if(mode==='auto')return;b.setPointerCapture(e.pointerId);drive(b.dataset.drive);driveTimer=setInterval(()=>drive(b.dataset.drive),250)};const release=()=>{if(driveTimer){clearInterval(driveTimer);driveTimer=null;drive('stop')}};b.onpointerup=release;b.onpointercancel=release;b.onlostpointercapture=release});
$('#stop').onclick=()=>drive('stop');window.onblur=()=>drive('stop');
document.querySelectorAll('[data-servo]').forEach(b=>b.onclick=async()=>{if(mode==='auto')return;$('#message').textContent='Measuring '+b.dataset.servo+'...';try{const r=await post('/api/servo',{direction:b.dataset.servo});$('#message').textContent=r.distanceCm<0?'No echo (treated as clear)':`${r.distanceCm} cm: ${r.result}`}catch(e){$('#message').textContent=e.message}refresh()});
refresh();setInterval(refresh,700);
</script></body></html>
)rawliteral";

main::main() {}
main::~main() {}

void main::begin()
{
  pinMode(L298N_IN1_PIN, OUTPUT);
  pinMode(L298N_IN2_PIN, OUTPUT);
  pinMode(L298N_IN3_PIN, OUTPUT);
  pinMode(L298N_IN4_PIN, OUTPUT);
  digitalWrite(L298N_IN1_PIN, LOW);
  digitalWrite(L298N_IN2_PIN, LOW);
  digitalWrite(L298N_IN3_PIN, LOW);
  digitalWrite(L298N_IN4_PIN, LOW);
  pinMode(BUZZER_PIN, OUTPUT);
  digitalWrite(BUZZER_PIN, LOW);
  lastBuzzerToggleAt = millis();

  ledController.initialize();
  servoController.begin();
  sensorDataCollector.begin();
  latestTemperatureC = sensorDataCollector.getTemperature();
  latestHumidityPercent = sensorDataCollector.getHumidity();
  displayController.begin();

  WiFi.mode(WIFI_STA);
  WiFi.begin(ssid, password);
  while (WiFi.waitForConnectResult() != WL_CONNECTED) {
    Serial.println("Connection Failed! Rebooting...");
    delay(5000);
    ESP.restart();
  }
  otaSetup(IN_BUILT_LED, "visiosphereai-esp32", password);
  // Use insecure TLS for testing
  espClient.setInsecure();
  client.setServer(mqtt_server, mqtt_port);
  mqttEventQueue = xQueueCreate(16, sizeof(MqttEvent));
  if (mqttEventQueue != nullptr) {
    BaseType_t taskCreated = xTaskCreatePinnedToCore(
      mqttTaskEntry,
      "mqttWorker",
      10240,
      this,
      1,
      nullptr,
      0);
    if (taskCreated != pdPASS) {
      Serial.println("MQTT worker task failed to start");
      vQueueDelete(mqttEventQueue);
      mqttEventQueue = nullptr;
    }
  } else {
    Serial.println("MQTT event queue allocation failed");
  }
  startWebServer();
  lastPeriodicMqttAt = millis();
  enqueueMqttEvent("boot");
  Serial.print("Boat control page: http://");
  Serial.println(WiFi.localIP());
}

void main::loop()
{
  constexpr uint32_t DISPLAY_UPDATE_INTERVAL_MS = 1000;
  constexpr uint32_t DISTANCE_SAMPLE_INTERVAL_MS = 60;
  constexpr uint32_t SERVO_SETTLE_TIME_MS = 300;
  constexpr uint32_t TURN_DURATION_MS = 350;
  constexpr uint32_t REVERSE_DURATION_MS = 250;
  constexpr uint32_t MANUAL_MOTOR_TIMEOUT_MS = 900;
  constexpr uint8_t SIDE_AVERAGE_SAMPLES = 3;

  otaHandle();
  webServer.handleClient();
  if (!otaInProgress && millis() - lastBlinkTime > 1000) {
    ledState = ledState == LOW ? HIGH : LOW;
    digitalWrite(otaLedPin, ledState);
    digitalWrite(BLINK_GREEN_LED_PIN, ledState);
    lastBlinkTime = millis();
  }
  uint32_t now = millis();

  if (controlMode == ControlMode::AUTONOMOUS) {
  switch (obstacleScanPhase) {
    case ObstacleScanPhase::IDLE:
      if (now - lastDistanceSampleAt >= DISTANCE_SAMPLE_INTERVAL_MS) {
        long reading = sensorDataCollector.getObjectDistance();
        frontSamples[frontSampleCount] = reading == ULTRASONIC_NO_ECHO
          ? ULTRASONIC_MAX_RANGE_CM
          : reading;
        if (reading == ULTRASONIC_NO_ECHO) {
          ++frontNoEchoCount;
        }
        ++frontSampleCount;
        lastDistanceSampleAt = now;

        if (frontSampleCount >= 3) {
          long filteredDistance = medianOfThree(
            frontSamples[0], frontSamples[1], frontSamples[2]);
          distance_c = frontNoEchoCount == 3
            ? ULTRASONIC_NO_ECHO
            : filteredDistance;
          frontSampleCount = 0;
          frontNoEchoCount = 0;

          if (filteredDistance < FRONT_OBSTACLE_DISTANCE_CM) {
            frontClearReadings = 0;
            setMovement(RobotState::STOP);
            sideSampleCount = 0;
            sideDistanceTotal = 0;
            servoState = ServoState::LEFT;
            servoController.left();
            obstacleScanPhase = ObstacleScanPhase::WAIT_LEFT;
            scanPhaseStartedAt = now;
          } else if (filteredDistance >= FRONT_CLEAR_DISTANCE_CM) {
            if (frontClearReadings < 2) {
              ++frontClearReadings;
            }
            if (frontClearReadings >= 2) {
              setMovement(RobotState::FORWARD);
            }
          } else {
            frontClearReadings = 0;
          }
        }
      }
      break;

    case ObstacleScanPhase::WAIT_LEFT:
      if (now - scanPhaseStartedAt >= SERVO_SETTLE_TIME_MS) {
        obstacleScanPhase = ObstacleScanPhase::SAMPLE_LEFT;
      }
      break;

    case ObstacleScanPhase::SAMPLE_LEFT:
      if (now - lastDistanceSampleAt >= DISTANCE_SAMPLE_INTERVAL_MS) {
        long sideDistance = sensorDataCollector.getObjectDistance();
        sideDistanceTotal += sideDistance == ULTRASONIC_NO_ECHO
          ? ULTRASONIC_MAX_RANGE_CM
          : sideDistance;
        lastDistanceSampleAt = now;
        if (++sideSampleCount >= SIDE_AVERAGE_SAMPLES) {
          distance_l = sideDistanceTotal / sideSampleCount;
          sideDistanceTotal = 0;
          sideSampleCount = 0;
          servoState = ServoState::RIGHT;
          servoController.right();
          obstacleScanPhase = ObstacleScanPhase::WAIT_RIGHT;
          scanPhaseStartedAt = now;
        }
      }
      break;

    case ObstacleScanPhase::WAIT_RIGHT:
      if (now - scanPhaseStartedAt >= SERVO_SETTLE_TIME_MS) {
        obstacleScanPhase = ObstacleScanPhase::SAMPLE_RIGHT;
      }
      break;

    case ObstacleScanPhase::SAMPLE_RIGHT:
      if (now - lastDistanceSampleAt >= DISTANCE_SAMPLE_INTERVAL_MS) {
        long sideDistance = sensorDataCollector.getObjectDistance();
        sideDistanceTotal += sideDistance == ULTRASONIC_NO_ECHO
          ? ULTRASONIC_MAX_RANGE_CM
          : sideDistance;
        lastDistanceSampleAt = now;
        if (++sideSampleCount >= SIDE_AVERAGE_SAMPLES) {
          distance_r = sideDistanceTotal / sideSampleCount;
          sideDistanceTotal = 0;
          sideSampleCount = 0;
          servoState = ServoState::CENTER;
          servoController.center();

          if (distance_l >= SIDE_CLEARANCE_DISTANCE_CM ||
              distance_r >= SIDE_CLEARANCE_DISTANCE_CM) {
            setMovement(distance_l >= distance_r
              ? RobotState::LEFT
              : RobotState::RIGHT);
            obstacleScanPhase = ObstacleScanPhase::TURNING;
          } else {
            setMovement(RobotState::BACKWARD);
            obstacleScanPhase = ObstacleScanPhase::REVERSING;
          }
          scanPhaseStartedAt = now;
        }
      }
      break;

    case ObstacleScanPhase::TURNING:
      if (now - scanPhaseStartedAt >= TURN_DURATION_MS) {
        setMovement(RobotState::STOP);
        obstacleScanPhase = ObstacleScanPhase::IDLE;
        lastDistanceSampleAt = now;
      }
      break;

    case ObstacleScanPhase::REVERSING:
      if (now - scanPhaseStartedAt >= REVERSE_DURATION_MS) {
        setMovement(RobotState::STOP);
        obstacleScanPhase = ObstacleScanPhase::IDLE;
        lastDistanceSampleAt = now;
      }
      break;
  }
  } else if (controlMode == ControlMode::MANUAL &&
             state != RobotState::STOP &&
             now - lastManualMotorCommandAt >= MANUAL_MOTOR_TIMEOUT_MS) {
    setMovement(RobotState::STOP);
    safetyMessage = "Manual input timed out; stopped";
  }

  updateBuzzer(millis());

  latestTemperatureC = sensorDataCollector.getTemperature();
  latestHumidityPercent = sensorDataCollector.getHumidity();
  uint32_t telemetryNow = millis();
  if (telemetryNow - lastPeriodicMqttAt >= MQTT_PERIODIC_INTERVAL_MS) {
    lastPeriodicMqttAt = telemetryNow;
    enqueueMqttEvent("periodic");
  }
  if (now - lastDisplayUpdate >= DISPLAY_UPDATE_INTERVAL_MS) {
    displayController.update(
      state,
      distance_c,
      latestTemperatureC,
      latestHumidityPercent);
    lastDisplayUpdate = now;
  }
}

{
  bool scanning = obstacleScanPhase == ObstacleScanPhase::WAIT_LEFT ||
    obstacleScanPhase == ObstacleScanPhase::SAMPLE_LEFT ||
    obstacleScanPhase == ObstacleScanPhase::WAIT_RIGHT ||
    obstacleScanPhase == ObstacleScanPhase::SAMPLE_RIGHT;
  bool movingForAvoidance = state == RobotState::BACKWARD ||
    state == RobotState::LEFT ||
    state == RobotState::RIGHT;

  if (!scanning && !movingForAvoidance) {
    digitalWrite(BUZZER_PIN, LOW);
    buzzerIsOn = false;
    lastBuzzerToggleAt = now;
    return;
  }

  uint32_t interval = buzzerIsOn
    ? BUZZER_BEEP_ON_MS
    : BUZZER_BEEP_OFF_MS;
  if (now - lastBuzzerToggleAt >= interval) {
    buzzerIsOn = !buzzerIsOn;
    digitalWrite(BUZZER_PIN, buzzerIsOn ? HIGH : LOW);
    lastBuzzerToggleAt = now;
  }
}

void main::setMovement(RobotState movement)
{
  if (state == movement) {
    return;
  }

  state = movement;
  switch (movement) {
    case RobotState::FORWARD:
      digitalWrite(L298N_IN1_PIN, HIGH);
      digitalWrite(L298N_IN2_PIN, LOW);
      digitalWrite(L298N_IN3_PIN, LOW);
      digitalWrite(L298N_IN4_PIN, HIGH);
      break;
    case RobotState::BACKWARD:
      digitalWrite(L298N_IN1_PIN, LOW);
      digitalWrite(L298N_IN2_PIN, HIGH);
      digitalWrite(L298N_IN3_PIN, HIGH);
      digitalWrite(L298N_IN4_PIN, LOW);
      break;
    case RobotState::LEFT:
      digitalWrite(L298N_IN1_PIN, LOW);
      digitalWrite(L298N_IN2_PIN, HIGH);
      digitalWrite(L298N_IN3_PIN, LOW);
      digitalWrite(L298N_IN4_PIN, HIGH);
      break;
    case RobotState::RIGHT:
      digitalWrite(L298N_IN1_PIN, HIGH);
      digitalWrite(L298N_IN2_PIN, LOW);
      digitalWrite(L298N_IN3_PIN, HIGH);
      digitalWrite(L298N_IN4_PIN, LOW);
      break;
    case RobotState::STOP:
      digitalWrite(L298N_IN1_PIN, LOW);
      digitalWrite(L298N_IN2_PIN, LOW);
      digitalWrite(L298N_IN3_PIN, LOW);
      digitalWrite(L298N_IN4_PIN, LOW);
      break;
  }

  ledController.updateRobotState(state);
  enqueueMqttEvent("movement");
}

void main::startWebServer()
{
  webServer.on("/", HTTP_GET, [this]() {
    webServer.send_P(200, "text/html", CONTROL_PAGE);
  });
  webServer.on("/api/status", HTTP_GET, [this]() {
    handleStatusRequest();
  });
  webServer.on("/api/mode", HTTP_POST, [this]() {
    handleModeRequest();
  });
  webServer.on("/api/motor", HTTP_POST, [this]() {
    handleMotorRequest();
  });
  webServer.on("/api/servo", HTTP_POST, [this]() {
    handleServoRequest();
  });
  webServer.onNotFound([this]() {
    webServer.send(404, "application/json", "{\"error\":\"Not found\"}");
  });
  webServer.begin();
}

void main::handleModeRequest()
{
  String requestedMode = webServer.arg("mode");
  if (requestedMode != "manual" && requestedMode != "auto") {
    webServer.send(400, "application/json", "{\"error\":\"mode must be auto or manual\"}");
    return;
  }

  ControlMode requestedControlMode = ControlMode::AUTONOMOUS;
  if (requestedMode == "manual") {
    requestedControlMode = ControlMode::MANUAL;
  }
  setControlMode(requestedControlMode);

  String response = "{\"mode\":\"" + requestedMode + "\"}";
  webServer.send(200, "application/json", response);
}

void main::setControlMode(ControlMode mode)
{
  setMovement(RobotState::STOP);
  servoController.center();
  servoState = ServoState::CENTER;
  obstacleScanPhase = ObstacleScanPhase::IDLE;
  frontSampleCount = 0;
  frontNoEchoCount = 0;
  frontClearReadings = 0;
  sideSampleCount = 0;
  lastDistanceSampleAt = millis();
  controlMode = mode;
  lastManualMotorCommandAt = millis();
  lastManualSafetyCheckAt = millis();
  safetyMessage = mode == ControlMode::AUTONOMOUS
    ? "Autonomous mode enabled"
    : "Manual mode enabled";
  enqueueMqttEvent("mode_changed");
}

void main::handleMotorRequest()
{
  handleMotorCommand(webServer.arg("command"));
}

void main::handleMotorCommand(String command)
{
  if (controlMode == ControlMode::AUTONOMOUS) {
    webServer.send(409, "application/json", "{\"error\":\"select manual mode first\"}");
    return;
  }

  if (command == "stop") {
    setMovement(RobotState::STOP);
    lastManualMotorCommandAt = millis();
    safetyMessage = "Stopped";
    webServer.send(200, "application/json", "{\"ok\":true}");
    return;
  }
  if (command == "backward") {
    setMovement(RobotState::BACKWARD);
    lastManualMotorCommandAt = millis();
    safetyMessage = "Reverse has no rear sensor protection";
    webServer.send(200, "application/json",
      "{\"ok\":true,\"warning\":\"No rear sensor: reverse is not collision-protected\"}");
    return;
  }

  RobotState requestedMovement = RobotState::STOP;
  ServoState requestedServo = ServoState::CENTER;
  uint16_t clearanceDistance = FRONT_OBSTACLE_DISTANCE_CM;
  if (command == "forward") {
    requestedMovement = RobotState::FORWARD;
  } else if (command == "left") {
    requestedMovement = RobotState::LEFT;
    requestedServo = ServoState::LEFT;
    clearanceDistance = SIDE_CLEARANCE_DISTANCE_CM;
  } else if (command == "right") {
    requestedMovement = RobotState::RIGHT;
    requestedServo = ServoState::RIGHT;
    clearanceDistance = SIDE_CLEARANCE_DISTANCE_CM;
  } else {
    webServer.send(400, "application/json", "{\"error\":\"unknown motor command\"}");
    return;
  }

  uint32_t now = millis();
  if (state == requestedMovement && servoState == requestedServo &&
      now - lastManualSafetyCheckAt < 400) {
    lastManualMotorCommandAt = now;
    webServer.send(200, "application/json", "{\"ok\":true,\"rangeCheck\":\"recent\"}");
    return;
  }

  bool directionAlreadySet = state == requestedMovement &&
    servoState == requestedServo;
  if (!directionAlreadySet) {
    setMovement(RobotState::STOP);
  }
  if (servoState != requestedServo) {
    if (requestedServo == ServoState::LEFT) {
      servoController.left();
    } else if (requestedServo == ServoState::RIGHT) {
      servoController.right();
    } else {
      servoController.center();
    }
    servoState = requestedServo;
    delay(300);
  }

  long rangeSamples[3];
  uint8_t noEchoCount = 0;
  for (uint8_t sample = 0; sample < 3; ++sample) {
    long reading = sensorDataCollector.getObjectDistance();
    if (reading == ULTRASONIC_NO_ECHO) {
      ++noEchoCount;
      rangeSamples[sample] = ULTRASONIC_MAX_RANGE_CM;
    } else {
      rangeSamples[sample] = reading;
    }
    if (sample < 2) {
      delay(60);
    }
  }

  long filteredDistance = medianOfThree(
    rangeSamples[0], rangeSamples[1], rangeSamples[2]);
  distance_c = noEchoCount > 0 ? ULTRASONIC_NO_ECHO : filteredDistance;
  lastManualSafetyCheckAt = millis();

  bool forwardSensorUnavailable =
    requestedMovement == RobotState::FORWARD && noEchoCount > 0;
  if (forwardSensorUnavailable || filteredDistance < clearanceDistance) {
    setMovement(RobotState::STOP);
    String reason = forwardSensorUnavailable
      ? "sensor returned no echo; movement blocked"
      : String(command) + " blocked: object at " + String(filteredDistance) + " cm";
    safetyMessage = reason;
    String response = "{\"blocked\":true,\"error\":\"" + reason + "\"}";
    webServer.send(409, "application/json", response);
    return;
  }

  setMovement(requestedMovement);
  lastManualMotorCommandAt = millis();
  bool sideScanUnavailable = requestedMovement != RobotState::FORWARD && noEchoCount > 0;
  safetyMessage = sideScanUnavailable
    ? "Side scan returned no echo; manual " + command + " active"
    : "Path clear; " + command + " active";
  String response = sideScanUnavailable
    ? "{\"ok\":true,\"rangeCheck\":\"no_echo\",\"warning\":\"Side scan returned no echo; manual turn is not range-verified\"}"
    : "{\"ok\":true,\"rangeCheck\":\"clear\"}";
  webServer.send(200, "application/json", response);
}

void main::handleServoRequest()
{
  handleServoDirection(webServer.arg("direction"));
}

void main::handleServoDirection(String direction)
{
  if (controlMode == ControlMode::AUTONOMOUS) {
    webServer.send(409, "application/json", "{\"error\":\"select manual mode first\"}");
    return;
  }

  setMovement(RobotState::STOP);
  if (direction == "left") {
    servoController.left();
    servoState = ServoState::LEFT;
  } else if (direction == "right") {
    servoController.right();
    servoState = ServoState::RIGHT;
  } else if (direction == "straight") {
    servoController.center();
    servoState = ServoState::CENTER;
  } else {
    webServer.send(400, "application/json", "{\"error\":\"direction must be left, right, or straight\"}");
    return;
  }

  delay(300);

  long totalDistance = 0;
  uint8_t noEchoCount = 0;
  for (uint8_t sample = 0; sample < 3; ++sample) {
    long reading = sensorDataCollector.getObjectDistance();
    if (reading == ULTRASONIC_NO_ECHO) {
      ++noEchoCount;
      totalDistance += ULTRASONIC_MAX_RANGE_CM;
    } else {
      totalDistance += reading;
    }
    if (sample < 2) {
      delay(60);
    }
  }

  long measuredDistance = noEchoCount == 3
    ? ULTRASONIC_NO_ECHO
    : totalDistance / 3;
  distance_c = measuredDistance;
  String result = measuredDistance == ULTRASONIC_NO_ECHO
    ? "no_echo"
    : measuredDistance < FRONT_OBSTACLE_DISTANCE_CM
      ? "object"
      : "clear";
  String response = "{\"servo\":\"" + direction + "\",\"distanceCm\":" +
    String(measuredDistance) + ",\"result\":\"" + result + "\"}";
  enqueueMqttEvent("sensor_reading");
  webServer.send(200, "application/json", response);
}

void main::handleStatusRequest()
{
  const char* mode = "auto";
  if (controlMode == ControlMode::MANUAL) {
    mode = "manual";
  }
  const char* movement = "stop";
  switch (state) {
    case RobotState::FORWARD: movement = "forward"; break;
    case RobotState::BACKWARD: movement = "backward"; break;
    case RobotState::LEFT: movement = "left"; break;
    case RobotState::RIGHT: movement = "right"; break;
    case RobotState::STOP: movement = "stop"; break;
  }

  String response = "{\"mode\":\"" + String(mode) + "\",\"movement\":\"" +
    movement + "\",\"distanceCm\":" + String(distance_c) +
    ",\"message\":\"" + safetyMessage + "\"}";
  webServer.send(200, "application/json", response);
}


void main::enqueueMqttEvent(const char* eventName)
{
  if (mqttEventQueue == nullptr) {
    return;
  }

  MqttEvent event{};
  strncpy(event.event, eventName, sizeof(event.event) - 1);
  event.motorState = static_cast<uint8_t>(state);
  event.servoState = static_cast<uint8_t>(servoState);
  event.controlMode = static_cast<uint8_t>(controlMode);
  event.distanceCm = distance_c;
  event.temperatureC = latestTemperatureC;
  event.humidityPercent = latestHumidityPercent;
  if (xQueueSend(mqttEventQueue, &event, 0) != pdTRUE) {
    MqttEvent discardedEvent;
    xQueueReceive(mqttEventQueue, &discardedEvent, 0);
    xQueueSend(mqttEventQueue, &event, 0);
  }
}

void main::mqttTaskEntry(void* parameter)
{
  static_cast<main*>(parameter)->mqttTask();
}

void main::mqttTask()
{
  uint32_t lastAttempt = millis() - 5000;
  MqttEvent pendingEvent{};
  bool hasPendingEvent = false;

  for (;;) {
    if (WiFi.status() != WL_CONNECTED) {
      vTaskDelay(pdMS_TO_TICKS(250));
      continue;
    }

    if (!client.connected()) {
      uint32_t now = millis();
      if (now - lastAttempt >= 5000) {
        lastAttempt = now;
        String clientId = "ESP32-" + String(random(0xffff), HEX);
        if (client.connect(clientId.c_str(), mqtt_user, mqtt_password)) {
          Serial.println("MQTT connected");
        } else {
          Serial.print("MQTT connection failed, rc=");
          Serial.println(client.state());
        }
      }
      vTaskDelay(pdMS_TO_TICKS(20));
      continue;
    }

    client.loop();
    if (!hasPendingEvent) {
      hasPendingEvent = xQueueReceive(
        mqttEventQueue,
        &pendingEvent,
        pdMS_TO_TICKS(20)) == pdTRUE;
    }

    if (hasPendingEvent) {
      char payload[224];
      static const char* const movementNames[] = {
        "stop", "forward", "backward", "left", "right"
      };
      static const char* const servoNames[] = {
        "center", "left", "right"
      };
      static const char* const modeNames[] = {
        "auto", "manual"
      };
      const char* movementName = pendingEvent.motorState < 5
        ? movementNames[pendingEvent.motorState]
        : "unknown";
      const char* servoName = pendingEvent.servoState < 3
        ? servoNames[pendingEvent.servoState]
        : "unknown";
      const char* modeName = pendingEvent.controlMode < 2
        ? modeNames[pendingEvent.controlMode]
        : "unknown";
      snprintf(
        payload,
        sizeof(payload),
        "{\"event\":\"%s\",\"movement\":\"%s\",\"servo\":\"%s\",\"mode\":\"%s\",\"distanceCm\":%ld,\"temperatureC\":%.1f,\"humidityPercent\":%.1f}",
        pendingEvent.event,
        movementName,
        servoName,
        modeName,
        pendingEvent.distanceCm,
        pendingEvent.temperatureC,
        pendingEvent.humidityPercent);

      if (client.publish("esp32/sensors", payload)) {
        hasPendingEvent = false;
        Serial.println(payload);
      }
    }
    vTaskDelay(pdMS_TO_TICKS(10));
  }
}