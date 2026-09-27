#include "../include/main.h"

main::main() {}
main::~main() {}

void main::begin()
{
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
  ledController.initialize();
  servoController.begin();
  sensorDataCollector.begin();
  displayController.begin();
}

void main::loop()
{
  constexpr uint32_t SERVO_SETTLE_TIME_MS = 300;
  constexpr uint32_t DISPLAY_UPDATE_INTERVAL_MS = 1000;

  otaHandle();
  if (!otaInProgress && millis() - lastBlinkTime > 1000) {
    ledState = ledState == LOW ? HIGH : LOW;
    digitalWrite(otaLedPin, ledState);
    lastBlinkTime = millis();
    ledController.blinkLed(BLINK_GREEN_LED_PIN);
  }
  distance_c = sensorDataCollector.getObjectDistance();
  uint32_t now = millis();

  if (obstacleScanPhase == ObstacleScanPhase::IDLE) {
    if (distance_c < 10) {
      state = RobotState::STOP;
      servoState = ServoState::LEFT;
      servoController.left();
      obstacleScanPhase = ObstacleScanPhase::WAIT_LEFT;
      scanPhaseStartedAt = now;
    } else {
      state = RobotState::FORWARD;
    }
  } else if (obstacleScanPhase == ObstacleScanPhase::WAIT_LEFT &&
             now - scanPhaseStartedAt >= SERVO_SETTLE_TIME_MS) {
    distance_l = sensorDataCollector.getObjectDistance();
    servoState = ServoState::RIGHT;
    servoController.right();
    obstacleScanPhase = ObstacleScanPhase::WAIT_RIGHT;
    scanPhaseStartedAt = now;
  } else if (obstacleScanPhase == ObstacleScanPhase::WAIT_RIGHT &&
             now - scanPhaseStartedAt >= SERVO_SETTLE_TIME_MS) {
    distance_r = sensorDataCollector.getObjectDistance();
    servoState = ServoState::CENTER;
    servoController.center();
    state = distance_l < 12 && distance_r < 12
      ? RobotState::BACKWARD
      : RobotState::STOP;
    obstacleScanPhase = ObstacleScanPhase::IDLE;
  }

  ledController.updateRobotState(state);
  float temperature = sensorDataCollector.getTemperature();
  float humidity = sensorDataCollector.getHumidity();
  if (now - lastDisplayUpdate >= DISPLAY_UPDATE_INTERVAL_MS) {
    displayController.update(state, distance_c, temperature, humidity);
    lastDisplayUpdate = now;
  }
  publishMQTT(
    static_cast<MotorState>(state),
    static_cast<ServoState>(servoState),
    ledState == HIGH,
    temperature,
    humidity,
    distance_c);
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