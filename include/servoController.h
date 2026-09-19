#ifndef SERVO_CONTROLLER_H
#define SERVO_CONTROLLER_H
#include <Arduino.h>
#include <ESP32Servo.h>
#include "config.h"
#include "constants.h"

class ServoController
{
private:
    Servo servoMotor;
    uint8_t servoAngle;
    ServoState state;
    uint32_t change_robot_time = 0;
public:
    ServoController();
    ~ServoController();
    void left();
    void right();
    void center();
    void setServoAngle(uint8_t servoAngle);
    uint8_t getServoAngle() const;
    ServoState getServoState() const;
};

#endif