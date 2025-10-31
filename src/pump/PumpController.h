#ifndef PUMP_CONTROLLER_H
#define PUMP_CONTROLLER_H

#include <Arduino.h>

class PumpController
{
private:
    float flowRate = 30; // in ms per ml
    uint16_t timeToStopMillis = 0;
    uint16_t pumpPin;
public:
    PumpController(uint16_t pumpPin) {
        this->pumpPin = pumpPin;
        pinMode(pumpPin, OUTPUT);
        digitalWrite(pumpPin, LOW); // Ensure pump is off initially
    }

    void startPump() {
        digitalWrite(pumpPin, HIGH); // Activate pump
    }

    void stopPump() {
        digitalWrite(pumpPin, LOW); // Deactivate pump
    }

    void dispenseVolume(float volumeMiliLiters) {
        float timeToRunMillis = 30.0 * volumeMiliLiters; // Calculate time to run based on flow rate
        timeToStopMillis = millis() + timeToRunMillis;
        startPump();
    }

    void update() {
        if (timeToStopMillis != 0 && millis() >= timeToStopMillis) {
            stopPump();
            timeToStopMillis = 0; // Reset
        }
    }
};
#endif // PUMP_CONTROLLER_H