#ifndef SENSOR_DATA_H
#define SENSOR_DATA_H
#include <Arduino.h>
#include <DHT.h>
#include "config.h"
#include "constants.h"

class SensorDataCollector
{
public:
    SensorDataCollector();
    ~SensorDataCollector();
    void begin();
    long getObjectDistance();
    float getTemperature();
    float getHumidity();
private:
    DHT dht;
    uint8_t echoPin;
    uint8_t triggerPin;
    long distance;
    long duration;
    float temperature;
    float humidity;
    unsigned long lastEnvironmentRead;
};

#endif