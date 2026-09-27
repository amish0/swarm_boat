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

// --- L298N motor driver (ENA/ENB jumpers fitted) ---
constexpr uint8_t L298N_IN1_PIN = 25;
constexpr uint8_t L298N_IN2_PIN = 26;
constexpr uint8_t L298N_IN3_PIN = 27;
constexpr uint8_t L298N_IN4_PIN = 32;
constexpr uint16_t FRONT_OBSTACLE_DISTANCE_CM = 25;
constexpr uint16_t FRONT_CLEAR_DISTANCE_CM = 35;
constexpr uint16_t SIDE_CLEARANCE_DISTANCE_CM = 30;
constexpr long ULTRASONIC_NO_ECHO = -1;
constexpr uint16_t ULTRASONIC_MAX_RANGE_CM = 400;

// --- Ultrasonic sensor setup ---
#define TRIG_PIN 5
#define ECHO_PIN 18

// --- Temperature and humidity sensor setup (DHT22) ---
#define DHT_DATA_PIN 33
#define DHT_SENSOR_TYPE DHT11

// --- LED setup ---
#define RED_LED_PIN 15 // STOP
#define GREEN_LED_PIN  4   // steady ON for forward
#define BLINK_GREEN_LED_PIN 19 // blinking heartbeat
#define BLUE_LED_PIN 23 // backard

// --- OLED display setup (I2C SSD1306, 128x64) ---
#define OLED_SDA_PIN 21
#define OLED_SCL_PIN 22
#define OLED_WIDTH 128
#define OLED_HEIGHT 64
#define OLED_I2C_ADDRESS 0x3C

#endif