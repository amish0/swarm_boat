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
}

void main::loop()
{
  otaHandle();
  if (millis() - change_robot_time > 5000)
  {
    state = static_cast<RobotState>(random(0, 3));
    ledController.updateRobotState(state);
    change_robot_time = millis();
    Serial.println("Robot State-->");
    Serial.println(static_cast<int>(state));
  }

  if (!otaInProgress && millis() - lastBlinkTime > 1000) {
    ledState = ledState == LOW ? HIGH : LOW;
    digitalWrite(otaLedPin, ledState);
    Serial.println(otaInProgress);
    Serial.println(ledState);
    lastBlinkTime = millis();
    ledController.blinkLed(BLINK_GREEN_LED_PIN);
  }
  publishMQTT(
    static_cast<MotorState>(state),
    ledState == HIGH,
    28.5,
    65.2,
    20.4);

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

void main::publishMQTT(MotorState motorState, bool signalBlink, float temperature, float humidity, float distance)
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
      ",\"signalBlink\":" + String(signalBlink ? "true" : "false") +
      ",\"temperature\":" + String(temperature,1) +
      ",\"humidity\":" + String(humidity,1) +
      ",\"distance\":" + String(distance,1) +
      "}";

    client.publish("esp32/sensors", payload.c_str());
    Serial.println(payload);
  }
}