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
    setServoAngle(90);
    state = ServoState::CENTER;
}

void ServoController::left()
{
    setServoAngle(120);
    state = ServoState::LEFT;
}

void ServoController::right()
{
     setServoAngle(30);
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