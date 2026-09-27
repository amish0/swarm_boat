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
}

void main::loop()
{
  constexpr uint32_t DISPLAY_UPDATE_INTERVAL_MS = 1000;
  constexpr uint32_t DISTANCE_SAMPLE_INTERVAL_MS = 60;
  constexpr uint32_t SERVO_SETTLE_TIME_MS = 300;
  constexpr uint32_t TURN_DURATION_MS = 350;
  constexpr uint32_t REVERSE_DURATION_MS = 250;
  constexpr uint8_t SIDE_AVERAGE_SAMPLES = 3;

  otaHandle();
  if (!otaInProgress && millis() - lastBlinkTime > 1000) {
    ledState = ledState == LOW ? HIGH : LOW;
    digitalWrite(otaLedPin, ledState);
    digitalWrite(BLINK_GREEN_LED_PIN, ledState);
    lastBlinkTime = millis();
  }
  uint32_t now = millis();

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
      digitalWrite(L298N_IN3_PIN, HIGH);
      digitalWrite(L298N_IN4_PIN, LOW);
      break;
    case RobotState::BACKWARD:
      digitalWrite(L298N_IN1_PIN, LOW);
      digitalWrite(L298N_IN2_PIN, HIGH);
      digitalWrite(L298N_IN3_PIN, LOW);
      digitalWrite(L298N_IN4_PIN, HIGH);
      break;
    case RobotState::LEFT:
      digitalWrite(L298N_IN1_PIN, LOW);
      digitalWrite(L298N_IN2_PIN, HIGH);
      digitalWrite(L298N_IN3_PIN, HIGH);
      digitalWrite(L298N_IN4_PIN, LOW);
      break;
    case RobotState::RIGHT:
      digitalWrite(L298N_IN1_PIN, HIGH);
      digitalWrite(L298N_IN2_PIN, LOW);
      digitalWrite(L298N_IN3_PIN, LOW);
      digitalWrite(L298N_IN4_PIN, HIGH);
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