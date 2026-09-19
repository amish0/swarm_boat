#ifndef CONFIG_H
#define CONFIG_H

#include <Arduino.h>

extern uint8_t otaLedPin;
extern uint32_t lastOtaTime;
extern uint32_t lastBlinkTime;
extern int ledState;
extern bool otaInProgress;


constexpr uint8_t IN_BUILT_LED = 2;
// Define pin 
// --- Servo setup ---
#define SERVO_PIN 13

// --- Ultrasonic sensor setup ---
#define TRIG_PIN 5
#define ECHO_PIN 18

// --- LED setup ---
#define RED_LED_PIN 15 // STOP
#define GREEN_LED_PIN  4   // steady ON for forward
#define BLINK_GREEN_LED_PIN 21 // blinking heartbeat
#define BLUE_LED_PIN 23 // backard

#endif