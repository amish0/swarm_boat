#ifndef MAIN_H
#define MAIN_H
#include <Arduino.h>
#include <WiFiClientSecure.h>
#include <PubSubClient.h>
#include <WebServer.h>
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
    enum class ObstacleScanPhase
    {
        IDLE,
        WAIT_LEFT,
        SAMPLE_LEFT,
        WAIT_RIGHT,
        SAMPLE_RIGHT,
        TURNING,
        REVERSING
    };

    enum class ControlMode
    {
        AUTONOMOUS,
        MANUAL
    };

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
    WebServer webServer{80};
    ControlMode controlMode = ControlMode::AUTONOMOUS;
    uint32_t change_robot_time = 0;
    unsigned long lastMsg = 0;
    uint32_t lastReconnectAttempt = 0;
    long distance_c = 0;
    long distance_l = 0;
    long distance_r = 0;
    ObstacleScanPhase obstacleScanPhase = ObstacleScanPhase::IDLE;
    uint32_t scanPhaseStartedAt = 0;
    uint32_t lastDistanceSampleAt = 0;
    long sideDistanceTotal = 0;
    long frontSamples[3] = {0, 0, 0};
    uint8_t sideSampleCount = 0;
    uint8_t frontSampleCount = 0;
    uint8_t frontNoEchoCount = 0;
    uint8_t frontClearReadings = 0;
    bool buzzerIsOn = false;
    uint32_t lastBuzzerToggleAt = 0;
    uint32_t lastDisplayUpdate = 0;
    uint32_t lastManualMotorCommandAt = 0;

    void setMovement(RobotState movement);
    void updateBuzzer(uint32_t now);
    void startWebServer();
    void handleModeRequest();
    void handleMotorRequest();
    void handleServoRequest();
    void handleStatusRequest();
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