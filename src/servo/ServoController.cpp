#include <Arduino.h>
#include <Servo.h>
#include "servo/ServoController.h"

ServoController::ServoController(int controlPin)
{
    this->controlPin = controlPin;
}

void ServoController::setAngle(int angle)
{
    // if (!servo.attached()) servo.attach(this->controlPin);

    // servo.write(angle);
    // relaxTime = millis() + 1000;
}

void ServoController::update()
{
    // if (relaxTime != 0 && millis() >= relaxTime) {
    //     relax();
    //     relaxTime = 0;
    // }
}