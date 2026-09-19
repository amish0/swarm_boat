#include <Arduino.h>
#include "include/main.h"
// #include "include/config.h"
// #include "include/ota.h"
// #include "include/version.h"
// #include "include/ledcontroller.h"
// #include "include/constants.h"

// const char* ssid = "Airtel_amish_4535";
// const char* password = "Amish@123";

// constexpr uint8_t IN_BUILT_LED = 2;

// LedController led_controller;
// RobotState state = RobotState::STOP;

// uint32_t change_robot_time = 0;

main main_c;

void setup() {
  Serial.begin(115200);
  Serial.println("Booting firmware " FIRMWARE_VERSION);
  main_c.begin();
  // WiFi.mode(WIFI_STA);
  // WiFi.begin(ssid, password);
  // while (WiFi.waitForConnectResult() != WL_CONNECTED) {
  //   Serial.println("Connection Failed! Rebooting...");
  //   delay(5000);
  //   ESP.restart();
  // }

  // otaSetup(IN_BUILT_LED, "visiosphereai-esp32", password);
  // led_controller.initialize();
}

void loop() {
  main_c.loop();
  // otaHandle();
  // if (millis() - change_robot_time > 5000)
  // {
  //   state = static_cast<RobotState>(random(0, 3));
  //   led_controller.updateRobotState(state);
  //   change_robot_time = millis();
  //   Serial.println("Robot State-->");
  //   Serial.println(static_cast<int>(state));
  // }

  // if (!otaInProgress && millis() - lastBlinkTime > 1000) {
  //   ledState = ledState == LOW ? HIGH : LOW;
  //   digitalWrite(otaLedPin, ledState);
  //   Serial.println(otaInProgress);
  //   Serial.println(ledState);
  //   lastBlinkTime = millis();
  //   led_controller.blinkLed(BLINK_GREEN_LED_PIN);
  // }
}
