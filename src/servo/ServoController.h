#ifndef SERVO_CONTROLLER_H
#define SERVO_CONTROLLER_H
#include <Arduino.h>
#include <Servo.h>

class ServoController {
public:
    ServoController(int controlPin);
    void open() {
        setAngle(90); // Default to middle position
    }
    void close() {
        setAngle(0); // Default to closed position
    }
    void update();
    private:
    void relax() {
        servo.detach();
    }
    void setAngle(int angle);
    Servo servo;
    int controlPin;
    uint32_t relaxTime = 0;
};
#endif // SERVO_CONTROLLER_H