#ifndef CONFIG_H
#define CONFIG_H

#include <Arduino.h>

extern uint8_t otaLedPin;
extern uint32_t lastOtaTime;
extern uint32_t lastBlinkTime;
extern int ledState;
extern bool otaInProgress;

#endif