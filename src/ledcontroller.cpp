#include "../include/ledcontroller.h"

LedController::LedController() {}

void LedController::initialize()
{
    pinMode(RED_LED_PIN, OUTPUT);
    pinMode(GREEN_LED_PIN, OUTPUT);
    pinMode(BLUE_LED_PIN, OUTPUT);
    pinMode(BLINK_GREEN_LED_PIN, OUTPUT);

    digitalWrite(RED_LED_PIN, LOW);
    digitalWrite(GREEN_LED_PIN, LOW);
    digitalWrite(BLUE_LED_PIN, LOW);
    digitalWrite(BLINK_GREEN_LED_PIN, LOW);
    digitalWrite(BLINK_GREEN_LED_PIN, HIGH);

    actionLed();
}

void LedController::turnLedOff(int pin)
{
    digitalWrite(pin, LOW);
}

void LedController::turnLedOn(int pin)
{
    digitalWrite(pin, HIGH);
}

void LedController::blinkLed(int pin)
{
    digitalWrite(pin, LOW);
    delay(200);
    digitalWrite(pin, HIGH);
}

void LedController::actionLed()
{
    switch (state)
    {
    case RobotState::STOP:
        turnLedOff(BLUE_LED_PIN);
        turnLedOff(GREEN_LED_PIN);
        turnLedOn(RED_LED_PIN);
        // blinkLed(BLINK_GREEN_LED_PIN);
        Serial.print("STOP");
        break;
    case RobotState::FORWARD:
        turnLedOff(BLUE_LED_PIN);
        turnLedOn(GREEN_LED_PIN);
        turnLedOff(RED_LED_PIN);
        // blinkLed(BLINK_GREEN_LED_PIN);
        Serial.print("FORWARD");
        break;
    case RobotState::BACKWARD:
        turnLedOn(BLUE_LED_PIN);
        turnLedOff(GREEN_LED_PIN);
        turnLedOff(RED_LED_PIN);
        // blinkLed(BLINK_GREEN_LED_PIN);
        Serial.print("BACKWARD");
        break;
    default:
        Serial.print("NA");
        break;
    }

}

void LedController::updateRobotState(RobotState state)
{ 
    this->state = state;
    actionLed();
}