#ifndef LED_CONTROLLER_H
#define LED_CONTROLLER_H

#include <Arduino.h>
#include "config.h"
#include "constants.h"

// LED Controller class
class LedController {
private:
    static const int BLINK_DURATION = 200;  // milliseconds
    RobotState state = RobotState::STOP;
public:
    // Constructor
    LedController();
    void initialize();
    void turnLedOn(int pin);
    void turnLedOff(int pin);
    void blinkLed(int pin);
    void updateRobotState(RobotState state);
    void actionLed();
};

#endif // LED_CONTROLLER_H
