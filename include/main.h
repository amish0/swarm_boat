#ifndef MAIN_H
#define MAIN_H
#include <Arduino.h>
#include <WiFiClientSecure.h>
#include <PubSubClient.h>
#include "config.h"
#include "constants.h"
#include "ota.h"
#include "ledcontroller.h"
#include "servoController.h"
#include "sensorData.h"
#include "displayController.h"
#include "version.h"

class main
{
private:
    LedController ledController;
    ServoController servoController;
    SensorDataCollector sensorDataCollector;
    DisplayController displayController;
    const char* ssid = "Airtel_amish_4535";
    const char* password = "Amish@123";
    const char* mqtt_server = "24b5f8e1ba974706b07f689402ddb9ef.s1.eu.hivemq.cloud";
    const int mqtt_port = 8883;
    const char* mqtt_user = "ai_boat";
    const char* mqtt_password = "Test@123";

    WiFiClientSecure espClient;
    PubSubClient client{espClient};
    uint32_t change_robot_time = 0;
    unsigned long lastMsg = 0;
    long distance_c = 0;
    long distance_l = 0;
    long distance_r = 0;
public:
    RobotState state = RobotState::STOP;
    ServoState servoState = ServoState::CENTER;
    main();
    ~main();
    void begin();
    void loop();
    void reconnectMQTT();
    void publishMQTT(MotorState motorState, ServoState servoState, bool signalBlink, float temperature, float humidity, float distance);
};

#endif