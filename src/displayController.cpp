#include "../include/displayController.h"
#include "../include/config.h"

DisplayController::DisplayController()
    : display(OLED_WIDTH, OLED_HEIGHT, &Wire, -1), available(false)
{
}

void DisplayController::begin()
{
    Wire.begin(OLED_SDA_PIN, OLED_SCL_PIN);
    available = display.begin(SSD1306_SWITCHCAPVCC, OLED_I2C_ADDRESS);

    if (!available) {
        Serial.println("OLED display not found");
        return;
    }

    display.clearDisplay();
    display.setTextColor(SSD1306_WHITE);
    display.setTextSize(1);
    display.setCursor(0, 0);
    display.println("hellow XE v0.0.0");
    display.display();
}

void DisplayController::update(RobotState state, long distance, float temperature, float humidity)
{
    if (!available) {
        return;
    }

    display.clearDisplay();
    display.setCursor(0, 0);
    display.println("hellow XE v0.0.0");

    display.print("Status: ");
    switch (state) {
        case RobotState::FORWARD:
            display.println("FORWARD");
            break;
        case RobotState::BACKWARD:
            display.println("BACKWARD");
            break;
        default:
            display.println("STOP");
            break;
    }

    if (distance < 10) {
        display.println("Object: DETECTED");
        display.print("Distance: ");
        display.print(distance);
        display.println(" cm");
    } else {
        display.println("Object: CLEAR");
        display.println("Distance: --");
    }

    display.print("Temp: ");
    display.print(temperature, 1);
    display.println(" C");
    display.print("Humidity: ");
    display.print(humidity, 1);
    display.println(" %");
    display.display();
}