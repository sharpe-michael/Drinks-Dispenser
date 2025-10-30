#include <Arduino.h>

class PumpController
{
private:
    float flowRate = 1.5; // in liters per minute
    uint16_t timeToStopMillis = 0;
public:
    PumpController(uint16_t pumpPin) {
        pinMode(pumpPin, OUTPUT);
        digitalWrite(pumpPin, LOW); // Ensure pump is off initially
    }

    void startPump(uint16_t pumpPin) {
        digitalWrite(pumpPin, HIGH); // Activate pump
    }

    void stopPump(uint16_t pumpPin) {
        digitalWrite(pumpPin, LOW); // Deactivate pump
    }

    void dispenseVolume(uint16_t pumpPin, float volumeLiters) {
        float timeToRunMinutes = volumeLiters / flowRate; // in minutes
        unsigned long timeToRunMillis = timeToRunMinutes * 60 * 1000; // convert to milliseconds
        timeToStopMillis = millis() + timeToRunMillis;
        startPump(pumpPin);
    }

    void update(uint16_t pumpPin) {
        if (timeToStopMillis != 0 && millis() >= timeToStopMillis) {
            stopPump(pumpPin);
            timeToStopMillis = 0; // Reset
        }
    }
};