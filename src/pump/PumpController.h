#ifndef PUMP_CONTROLLER_H
#define PUMP_CONTROLLER_H

#include <Arduino.h>

class PumpController
{
private:
    float flowRate = 30; // in ms per ml
    uint16_t pumpPin;
    public:
    uint32_t timeToStopMillis = 0;
    uint32_t timeToStartMillis = 0;

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

    uint32_t dispenseVolume(float volumeMiliLiters, uint32_t delayBeforeStartMillis = 0) {
        float timeToRunMillis = 30.0 * volumeMiliLiters; // Calculate time to run based on flow rate
        timeToStartMillis = millis() + delayBeforeStartMillis;
        timeToStopMillis = timeToStartMillis + timeToRunMillis;
        if (delayBeforeStartMillis == 0) startPump();

        return timeToRunMillis;
    }

    void update() {
        if (timeToStartMillis != 0 && millis() >= timeToStartMillis) {
            startPump();
            timeToStartMillis = 0; // Reset
        }
        if (timeToStopMillis != 0 && millis() >= timeToStopMillis) {
            stopPump();
            timeToStopMillis = 0; // Reset
        }
    }
};
#endif // PUMP_CONTROLLER_H