#include "../include/main.h"
#include "../include/constants.h"

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
  sensorDataCollector.begin();
}

void main::loop()
{
  otaHandle();
//   if (millis() - change_robot_time > 5000)
//   {
//     state = static_cast<RobotState>(random(0, 3));
//     ledController.updateRobotState(state);
//     change_robot_time = millis();
//     Serial.println("Robot State-->");
//     Serial.println(static_cast<int>(state));
//   }

  if (!otaInProgress && millis() - lastBlinkTime > 1000) {
    ledState = ledState == LOW ? HIGH : LOW;
    digitalWrite(otaLedPin, ledState);
    Serial.println(otaInProgress);
    Serial.println(ledState);
    lastBlinkTime = millis();
    ledController.blinkLed(BLINK_GREEN_LED_PIN);
  }
//   if (servoState == ServoState::CENTER)
//   {
//     servoState = ServoState::LEFT;
//     servoController.left();
//   } else if (servoState == ServoState::LEFT)
//   {
//     servoState = ServoState::RIGHT;
//     servoController.right();
//   } else 
//   {
//     servoState = ServoState::CENTER;
//     servoController.center();
//   }
  
  distance_c = sensorDataCollector.getObjectDistance();
  Serial.println("distance-->");
  Serial.println(distance_c);
  if (distance_c < 10 )
  {
    state = RobotState::STOP;
    ledController.updateRobotState(state);
    change_robot_time = millis();
    Serial.println("Robot State-->");
    Serial.println(static_cast<int>(state));
    servoState = ServoState::LEFT;
    servoController.left();
    delay(1000);
    distance_l = sensorDataCollector.getObjectDistance();
    servoState = ServoState::RIGHT;
    servoController.right();
    delay(1000);
    distance_r = sensorDataCollector.getObjectDistance();
    servoState = ServoState::CENTER;
    servoController.center();

    if (distance_l < 12 && distance_r < 12 )
    {
      state = RobotState::BACKWARD;
      ledController.updateRobotState(state);
      change_robot_time = millis();
      Serial.println("Robot State-->");
      Serial.println(static_cast<int>(state));
    }
  } else
  {
    state = RobotState::FORWARD;
    ledController.updateRobotState(state);
    change_robot_time = millis();
    Serial.println("Robot State-->");
    Serial.println(static_cast<int>(state));
  }
  publishMQTT(
    static_cast<MotorState>(state),
    static_cast<ServoState>(servoState),
    ledState == HIGH,
    28.5,
    65.2,
    20.4);
//   delay(1000);
  
}


void main::reconnectMQTT()
{
  while (!client.connected()) {

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
      delay(5000);
    }
  }
}

void main::publishMQTT(MotorState motorState, ServoState servoState , bool signalBlink, float temperature, float humidity, float distance)
{
  if (!client.connected()) {
    reconnectMQTT();
  }

  client.loop();

  unsigned long now = millis();

  if (now - lastMsg > 5000) {
    lastMsg = now;

    String payload =
      "{\"motorState\":" + String(static_cast<int>(motorState)) +
      "\"Servo Position\":" + String(static_cast<int>(servoState)) +
      ",\"signalBlink\":" + String(signalBlink ? "true" : "false") +
      ",\"temperature\":" + String(temperature,1) +
      ",\"humidity\":" + String(humidity,1) +
      ",\"distance\":" + String(distance,1) +
      "}";

    client.publish("esp32/sensors", payload.c_str());
    Serial.println(payload);
  }
}