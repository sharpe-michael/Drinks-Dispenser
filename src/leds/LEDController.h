#ifndef LEDCONTROLLER_H
#define LEDCONTROLLER_H

#include <Arduino.h>
#include <FastLED.h>

enum LEDMode
{
    IDLE_LEDS,
    DISPENSING_LEDS,
    FINISHED_LEDS
};
class LEDController
{
public:
    LEDController();

    void begin();
    void setColor(int index, CRGB color);
    void show();
    void update();
    LEDMode mode; // Add mode member to track current state

private:
    CRGB *leds;
    uint32_t lastUpdate;
    int idleIndex;
    uint8_t rainbowHue;
    bool finishedOn;
};

#endif // LEDCONTROLLER_H