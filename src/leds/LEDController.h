#include <Arduino.h>
#include <FastLED.h>

class LEDController
{
public:
    LEDController(int dataPin, int numLEDs);

    void setColor(int index, CRGB color);
    void show();
    void update();

private:
    CRGB *leds;
    int numLEDs;
    int dataPin;
    enum LEDMode {
        IDLE,
        DISPENSING,
        FINISHED
    };
    LEDMode mode; // Add mode member to track current state
};