#include <Arduino.h>
#include <Servo.h>

class ServoController {
public:
    ServoController(int controlPin);
    void begin();
    void open() {
        setAngle(90); // Default to middle position
    }
    void close() {
        setAngle(0); // Default to closed position
    }
    private:
    void setAngle(int angle);
    Servo servo;
    int controlPin;
};