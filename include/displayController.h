#ifndef DISPLAY_CONTROLLER_H
#define DISPLAY_CONTROLLER_H

#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>
#include "constants.h"

class DisplayController
{
public:
    DisplayController();
    void begin();
    void update(RobotState state, long distance, float temperature, float humidity);

private:
    Adafruit_SSD1306 display;
    bool available;
};

#endif