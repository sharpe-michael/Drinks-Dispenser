#include <Arduino.h>
#include <FastLED.h>
#include "leds/LEDController.h"

#define DATA_PIN A0

// Helper variables for animation timing
static uint32_t lastUpdate = 0;
static int idleIndex = 0;
static uint8_t rainbowHue = 0;
static bool finishedOn = false;

LEDController::LEDController(int dataPin, int numLEDs) : dataPin(dataPin), numLEDs(numLEDs)
{
    leds = new CRGB[numLEDs];
    FastLED.addLeds<NEOPIXEL, DATA_PIN>(leds, numLEDs);
    FastLED.setBrightness(128); // Set initial brightness to 50%
    // Default to IDLE mode
    mode = IDLE;
}

void LEDController::setColor(int index, CRGB color)
{
    if (index >= 0 && index < numLEDs) {
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
    switch (mode) {
        case IDLE:
            // Slow blue color wipe
            if (now - lastUpdate > 100) {
                for (int i = 0; i < numLEDs; ++i) leds[i] = CRGB::Black;
                leds[idleIndex] = CRGB::Blue;
                idleIndex = (idleIndex + 1) % numLEDs;
                show();
                lastUpdate = now;
            }
            break;
        case DISPENSING:
            // Fast rainbow cycle
            if (now - lastUpdate > 30) {
                for (int i = 0; i < numLEDs; ++i) {
                    leds[i] = CHSV((rainbowHue + i * 8) % 255, 255, 255);
                }
                rainbowHue += 4;
                show();
                lastUpdate = now;
            }
            break;
        case FINISHED:
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