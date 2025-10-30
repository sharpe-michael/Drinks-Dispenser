#include <Arduino.h>
#include <Servo.h>
#include "servo/ServoController.h"

ServoController::ServoController(int controlPin) : controlPin(controlPin) {
}

void ServoController::begin() {
    servo.attach(controlPin);
}

void ServoController::setAngle(int angle) {
    servo.write(angle);
}