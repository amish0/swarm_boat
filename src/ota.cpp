#include "../include/ota.h"
#include "../include/config.h"

uint8_t otaLedPin;
uint32_t lastOtaTime = 0;
uint32_t lastBlinkTime = 0;
int ledState = LOW;
bool otaInProgress = false;

void otaSetup(uint8_t ledPin, const char* hostname, const char* password) {
  otaLedPin = ledPin;
  pinMode(otaLedPin, OUTPUT);
  digitalWrite(otaLedPin, LOW);

  ArduinoOTA.setHostname(hostname);
  ArduinoOTA.setPassword(password);

  ArduinoOTA
    .onStart([]() {
      String type = ArduinoOTA.getCommand() == U_FLASH ? "sketch" : "filesystem";
      otaInProgress = true;
      digitalWrite(otaLedPin, HIGH);
      Serial.println("Start updating " + type);
    })
    .onEnd([]() {
      otaInProgress = false;
      digitalWrite(otaLedPin, LOW);
      Serial.println("\nEnd");
    })
    .onProgress([](unsigned int progress, unsigned int total) {
      if (millis() - lastOtaTime > 500) {
        Serial.printf("Progress: %u%%\n", progress / (total / 100));
        lastOtaTime = millis();
        digitalWrite(otaLedPin, (progress / (total / 10)) % 2);
      }
    })
    .onError([](ota_error_t error) {
      Serial.printf("Error[%u]: ", error);
      if (error == OTA_AUTH_ERROR) {
        Serial.println("Auth Failed");
      } else if (error == OTA_BEGIN_ERROR) {
        Serial.println("Begin Failed");
      } else if (error == OTA_CONNECT_ERROR) {
        Serial.println("Connect Failed");
      } else if (error == OTA_RECEIVE_ERROR) {
        Serial.println("Receive Failed");
      } else if (error == OTA_END_ERROR) {
        Serial.println("End Failed");
      }
      otaInProgress = false;
      digitalWrite(otaLedPin, LOW);
    });

  ArduinoOTA.begin();

  Serial.println("Ready");
  Serial.print("IP address: ");
  Serial.println(WiFi.localIP());
}

void otaHandle() {
  ArduinoOTA.handle();
}
