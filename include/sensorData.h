#ifndef SENSOR_DATA_H
#define SENSOR_DATA_H
#include <Arduino.h>
#include "config.h"
#include "constants.h"

class SensorDataCollector
{
public:
    SensorDataCollector();
    ~SensorDataCollector();
    void begin();
    long getObjectDistance();
private:
    uint8_t echoPin;
    uint8_t triggerPin;
    long distance;
    long duration;
};

#endif