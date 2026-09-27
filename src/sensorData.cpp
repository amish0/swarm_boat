#include "../include/sensorData.h"

SensorDataCollector::SensorDataCollector()
    : dht(DHT_DATA_PIN, DHT_SENSOR_TYPE), distance(0), duration(0),
      temperature(28.5), humidity(65.2), lastEnvironmentRead(0)
{
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
    pinMode(DHT_DATA_PIN, INPUT_PULLUP);
    dht.begin();
    lastEnvironmentRead = millis() - 2000;
    Serial.print("DHT sensor initialized on GPIO ");
    Serial.println(DHT_DATA_PIN);
}

long SensorDataCollector::getObjectDistance()
{   
    digitalWrite(triggerPin, LOW);
    delayMicroseconds(2);
    digitalWrite(triggerPin, HIGH);
    delayMicroseconds(10);
    digitalWrite(triggerPin, LOW);
    duration = pulseIn(echoPin, HIGH, 30000UL);
    distance = duration * 0.034 / 2; // cm
    return distance;
}

float SensorDataCollector::getTemperature()
{
    if (millis() - lastEnvironmentRead >= 2000) {
        float newTemperature = dht.readTemperature();
        float newHumidity = dht.readHumidity();
        lastEnvironmentRead = millis();

        if (!isnan(newTemperature) && !isnan(newHumidity)) {
            temperature = newTemperature;
            humidity = newHumidity;
        } else {
            Serial.println("DHT sensor read failed; using last values");
        }
    }

    return temperature;
}

float SensorDataCollector::getHumidity()
{
    getTemperature();
    return humidity;
}