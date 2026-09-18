#include <Arduino.h>
#include "include/config.h"
#include "include/ota.h"
#include "include/version.h"

const char* ssid = "Airtel_amish_4535";
const char* password = "Amish@123";

constexpr uint8_t IN_BUILT_LED = 2;

void setup() {
  Serial.begin(115200);
  Serial.println("Booting firmware " FIRMWARE_VERSION);

  WiFi.mode(WIFI_STA);
  WiFi.begin(ssid, password);
  while (WiFi.waitForConnectResult() != WL_CONNECTED) {
    Serial.println("Connection Failed! Rebooting...");
    delay(5000);
    ESP.restart();
  }

  otaSetup(IN_BUILT_LED, "visiosphereai-esp32", password);
}

void loop() {
  otaHandle();
  if (!otaInProgress && millis() - lastBlinkTime > 1000) {
    ledState = ledState == LOW ? HIGH : LOW;
    digitalWrite(otaLedPin, ledState);
    Serial.println(otaInProgress);
    Serial.println(ledState);
    lastBlinkTime = millis();
  }
}
