#include "../include/servoController.h"

ServoController::ServoController()
{
    state = ServoState::CENTER;
    servoMotor.attach(SERVO_PIN);
    center();
}

ServoController::~ServoController() 
{
    center();
}

void ServoController::center()
{
    setServoAngle(45);
    state = ServoState::CENTER;
    Serial.println("Center--");
    Serial.println("45");
}

void ServoController::left()
{
    setServoAngle(15);
    state = ServoState::LEFT;
    Serial.println("Left--");
    Serial.println("15");
}

void ServoController::right()
{
     setServoAngle(70);
     Serial.println("Right--");
     Serial.println("75");
     state = ServoState::RIGHT;
}

void ServoController::setServoAngle(uint8_t servoAngle)
{
    servoMotor.write(servoAngle);
    this->servoAngle = servoAngle;
}

ServoState ServoController::getServoState() const
{
    return state;
}

uint8_t ServoController::getServoAngle() const
{
    return servoAngle;
}