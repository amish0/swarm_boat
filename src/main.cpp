#include "../include/main.h"

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
:root{color-scheme:light;--ink:#17212b;--muted:#61717c;--line:#cbd5d6;--paper:#f4f7f5;--panel:#fff;--teal:#087e78;--red:#b83232}
*{box-sizing:border-box}body{margin:0;background:var(--paper);color:var(--ink);font:16px/1.4 system-ui,sans-serif}
main{max-width:760px;margin:0 auto;padding:20px}header{display:flex;justify-content:space-between;align-items:center;border-bottom:1px solid var(--line);padding:6px 0 16px}
h1{font-size:24px;margin:0}.readout{display:grid;grid-template-columns:repeat(3,1fr);gap:10px;margin:18px 0}
.readout div{background:var(--panel);border:1px solid var(--line);padding:12px}.label{font-size:12px;text-transform:uppercase;color:var(--muted)}.value{font-size:20px;font-weight:650;margin-top:4px}
section{margin:18px 0}h2{font-size:15px;text-transform:uppercase;color:var(--muted);margin:0 0 9px}
.modes,.sensors{display:flex;gap:8px}.drive{display:grid;grid-template-columns:repeat(3,1fr);gap:8px;max-width:420px;margin:auto}
button{min-height:52px;border:1px solid var(--line);background:var(--panel);color:var(--ink);font:inherit;font-weight:650;padding:10px 14px;touch-action:none}
button.active,button:active{background:var(--teal);border-color:var(--teal);color:#fff}.stop{color:var(--red)}button:disabled{opacity:.42}
.drive .up{grid-column:2}.drive .left{grid-column:1;grid-row:2}.drive .stop{grid-column:2;grid-row:2}.drive .right{grid-column:3;grid-row:2}.drive .down{grid-column:2;grid-row:3}
#message{min-height:26px;color:var(--muted)}@media(max-width:520px){main{padding:14px}.readout{gap:6px}.readout div{padding:9px}.value{font-size:17px}.sensors button,.modes button{flex:1}}
</style>
</head>
<body><main>
<header><h1>Boat Control</h1><span id="connection">Connecting</span></header>
<div class="readout"><div><div class="label">Mode</div><div class="value" id="mode">--</div></div><div><div class="label">Movement</div><div class="value" id="movement">--</div></div><div><div class="label">Front range</div><div class="value" id="range">--</div></div></div>
<section><h2>Control mode</h2><div class="modes"><button id="auto">Auto</button><button id="manual">Manual</button></div></section>
<section><h2>Drive</h2><div class="drive">
<button class="up" data-drive="forward">Forward</button><button class="left" data-drive="left">Left</button><button class="stop" id="stop">Stop</button><button class="right" data-drive="right">Right</button><button class="down" data-drive="backward">Backward</button>
</div></section>
<section><h2>Sensor direction</h2><div class="sensors"><button data-servo="left">Left</button><button data-servo="straight">Straight</button><button data-servo="right">Right</button></div></section>
<p id="message" role="status"></p>
</main>
<script>
const $=s=>document.querySelector(s),post=(url,data)=>fetch(url,{method:'POST',headers:{'Content-Type':'application/x-www-form-urlencoded'},body:new URLSearchParams(data)}).then(async r=>{let j=await r.json();if(!r.ok)throw Error(j.error||'Request failed');return j});
let mode='auto',driveTimer=null;
async function refresh(){try{const r=await fetch('/api/status'),s=await r.json();mode=s.mode;$('#mode').textContent=mode.toUpperCase();$('#movement').textContent=s.movement;$('#range').textContent=s.distanceCm<0?'NO ECHO':`${s.distanceCm} cm`;$('#connection').textContent='Online';$('#auto').classList.toggle('active',mode==='auto');$('#manual').classList.toggle('active',mode==='manual');document.querySelectorAll('[data-drive],[data-servo],#stop').forEach(b=>b.disabled=mode!=='manual')}catch(e){$('#connection').textContent='Offline'}}
$('#auto').onclick=async()=>{try{await post('/api/mode',{mode:'auto'});$('#message').textContent='Autonomous control enabled'}catch(e){$('#message').textContent=e.message}refresh()};
$('#manual').onclick=async()=>{try{await post('/api/mode',{mode:'manual'});$('#message').textContent='Manual control enabled'}catch(e){$('#message').textContent=e.message}refresh()};
async function drive(command){try{await post('/api/motor',{command})}catch(e){$('#message').textContent=e.message}}
document.querySelectorAll('[data-drive]').forEach(b=>{b.onpointerdown=e=>{e.preventDefault();if(mode!=='manual')return;b.setPointerCapture(e.pointerId);drive(b.dataset.drive);driveTimer=setInterval(()=>drive(b.dataset.drive),250)};const release=()=>{if(driveTimer){clearInterval(driveTimer);driveTimer=null;drive('stop')}};b.onpointerup=release;b.onpointercancel=release;b.onlostpointercapture=release});
$('#stop').onclick=()=>drive('stop');window.onblur=()=>drive('stop');
document.querySelectorAll('[data-servo]').forEach(b=>b.onclick=async()=>{if(mode!=='manual')return;$('#message').textContent='Measuring '+b.dataset.servo+'...';try{const r=await post('/api/servo',{direction:b.dataset.servo});$('#message').textContent=r.distanceCm<0?'No echo (treated as clear)':`${r.distanceCm} cm: ${r.result}`}catch(e){$('#message').textContent=e.message}refresh()});
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
  startWebServer();
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
  } else if (state != RobotState::STOP &&
             now - lastManualMotorCommandAt >= MANUAL_MOTOR_TIMEOUT_MS) {
    setMovement(RobotState::STOP);
  }

  updateBuzzer(millis());

  float temperature = sensorDataCollector.getTemperature();
  float humidity = sensorDataCollector.getHumidity();
  if (now - lastDisplayUpdate >= DISPLAY_UPDATE_INTERVAL_MS) {
    displayController.update(state, distance_c, temperature, humidity);
    lastDisplayUpdate = now;
  }
  // publishMQTT(
  //   static_cast<MotorState>(state),
  //   static_cast<ServoState>(servoState),
  //   ledState == HIGH,
  //   temperature,
  //   humidity,
  //   distance_c);
}

void main::updateBuzzer(uint32_t now)
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
    webServer.send(400, "application/json", "{\"error\":\"mode must be manual or auto\"}");
    return;
  }

  setMovement(RobotState::STOP);
  servoController.center();
  servoState = ServoState::CENTER;
  obstacleScanPhase = ObstacleScanPhase::IDLE;
  frontSampleCount = 0;
  frontNoEchoCount = 0;
  frontClearReadings = 0;
  sideSampleCount = 0;
  lastDistanceSampleAt = millis();
  controlMode = requestedMode == "manual"
    ? ControlMode::MANUAL
    : ControlMode::AUTONOMOUS;
  lastManualMotorCommandAt = millis();

  String response = "{\"mode\":\"" + requestedMode + "\"}";
  webServer.send(200, "application/json", response);
}

void main::handleMotorRequest()
{
  if (controlMode != ControlMode::MANUAL) {
    webServer.send(409, "application/json", "{\"error\":\"enable manual mode first\"}");
    return;
  }

  String command = webServer.arg("command");
  if (command == "forward") {
    setMovement(RobotState::FORWARD);
  } else if (command == "backward") {
    setMovement(RobotState::BACKWARD);
  } else if (command == "left") {
    setMovement(RobotState::LEFT);
  } else if (command == "right") {
    setMovement(RobotState::RIGHT);
  } else if (command == "stop") {
    setMovement(RobotState::STOP);
  } else {
    webServer.send(400, "application/json", "{\"error\":\"unknown motor command\"}");
    return;
  }

  lastManualMotorCommandAt = millis();
  webServer.send(200, "application/json", "{\"ok\":true}");
}

void main::handleServoRequest()
{
  if (controlMode != ControlMode::MANUAL) {
    webServer.send(409, "application/json", "{\"error\":\"enable manual mode first\"}");
    return;
  }

  setMovement(RobotState::STOP);
  String direction = webServer.arg("direction");
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
  webServer.send(200, "application/json", response);
}

void main::handleStatusRequest()
{
  const char* mode = controlMode == ControlMode::MANUAL ? "manual" : "auto";
  const char* movement = "stop";
  switch (state) {
    case RobotState::FORWARD: movement = "forward"; break;
    case RobotState::BACKWARD: movement = "backward"; break;
    case RobotState::LEFT: movement = "left"; break;
    case RobotState::RIGHT: movement = "right"; break;
    case RobotState::STOP: movement = "stop"; break;
  }

  String response = "{\"mode\":\"" + String(mode) + "\",\"movement\":\"" +
    movement + "\",\"distanceCm\":" + String(distance_c) + "}";
  webServer.send(200, "application/json", response);
}


void main::reconnectMQTT()
{
  uint32_t now = millis();
  if (now - lastReconnectAttempt < 5000) {
    return;
  }
  lastReconnectAttempt = now;

  String clientId = "ESP32-" + String(random(0xffff), HEX);

  Serial.print("Connecting MQTT...");

  if (client.connect(
        clientId.c_str(),
        mqtt_user,
        mqtt_password))
  {
    Serial.println("Connected");
  }
  else
  {
    Serial.print("Failed, rc=");
    Serial.println(client.state());
  }
}

void main::publishMQTT(MotorState motorState, ServoState servoState , bool signalBlink, float temperature, float humidity, float distance)
{
  if (!client.connected()) {
    reconnectMQTT();
  }

  if (!client.connected()) {
    return;
  }

  client.loop();

  unsigned long now = millis();

  if (now - lastMsg > 5000) {
    lastMsg = now;

    String payload =
      "{\"motorState\":" + String(static_cast<int>(motorState)) +
      ",\"servoPosition\":" + String(static_cast<int>(servoState)) +
      ",\"signalBlink\":" + String(signalBlink ? "true" : "false") +
      ",\"temperature\":" + String(temperature,1) +
      ",\"humidity\":" + String(humidity,1) +
      ",\"distance\":" + String(distance,1) +
      "}";

    client.publish("esp32/sensors", payload.c_str());
    Serial.println(payload);
  }
}