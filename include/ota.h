#ifndef OTA_H
#define OTA_H

#include <Arduino.h>
#include <ESPmDNS.h>
// #include <NetworkUdp.h>
#include <WiFiUdp.h>
#include <ArduinoOTA.h>
#include <WiFi.h>


void otaSetup(uint8_t ledPin, const char* hostname, const char* password);
void otaHandle();

#endif
