#include "../include/sensorData.h"

SensorDataCollector::SensorDataCollector()
{
    distance = 0;
}

SensorDataCollector::~SensorDataCollector()
{
    distance = 0;
}

void SensorDataCollector::begin()
{   
    echoPin = ECHO_PIN;
    triggerPin = TRIG_PIN;
    pinMode(echoPin, INPUT);
    pinMode(triggerPin, OUTPUT);
}

long SensorDataCollector::getObjectDistance()
{   
    digitalWrite(triggerPin, LOW);
    delayMicroseconds(2);
    digitalWrite(triggerPin, HIGH);
    delayMicroseconds(10);
    digitalWrite(triggerPin, LOW);
    duration = pulseIn(echoPin, HIGH, 30000UL);
    Serial.println("duration-->");
    Serial.println(duration);
    distance = duration * 0.034 / 2; // cm
    return distance;
}