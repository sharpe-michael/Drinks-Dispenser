#include <Arduino.h>
#include <FastLED.h>
#include "leds/LEDController.h"

#define DATA_PIN A0
#define numLEDs 16

LEDController::LEDController()
{
    leds = new CRGB[numLEDs];
}

void LEDController::begin()
{
    FastLED.addLeds<NEOPIXEL, DATA_PIN>(leds, numLEDs);
    FastLED.setBrightness(50); // Set initial brightness to 50%
    // Default to IDLE mode
    mode = IDLE_LEDS;

    lastUpdate = millis();
    idleIndex = 0;
    rainbowHue = 0;
    finishedOn = false;
}

void LEDController::setColor(int index, CRGB color)
{
    if (index >= 0 && index < numLEDs)
    {
        leds[index] = color;
    }
}

void LEDController::show()
{
    FastLED.show();
}

void LEDController::update()
{
    uint32_t now = millis();

    if (nextUpdateTime !=0 && now > nextUpdateTime) {
        mode = nextUpdateMode;
        nextUpdateTime = 0;
    }

    switch (mode) {
        case IDLE_LEDS:
            // Slow blue color wipe
            if (now - lastUpdate > 100) {
                for (int i = 0; i < numLEDs; ++i) leds[i] = CRGB::Black;
                leds[idleIndex] = CRGB::OrangeRed;
                idleIndex = (idleIndex + 1) % numLEDs;
                show();
                lastUpdate = now;
            }
            break;
        case DISPENSING_LEDS:
            // Theater chase white
            if (now - lastUpdate > 30) {
                for (int i = 0; i < numLEDs; ++i) {
                    if ((i + (now / 100)) % 3 == 0) {
                        leds[i] = CRGB::White;
                    } else {
                        leds[i] = CRGB::Black;
                    }
                }
                show();
                lastUpdate = now;
            }
            break;
        case FINISHED_LEDS:
            // All LEDs flash green
            if (now - lastUpdate > 250) {
                finishedOn = !finishedOn;
                for (int i = 0; i < numLEDs; ++i) {
                    leds[i] = finishedOn ? CRGB::Green : CRGB::Black;
                }
                show();
                lastUpdate = now;
            }
            break;
    }
}