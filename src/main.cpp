#include <Arduino.h>
#include "screen/ScreenController.h"

// For the Adafruit shield, these are the default.
#define TFT_DC 9
#define TFT_CS 10
#define TFT_RST 8
#define TOUCH_CS 7

ScreenController screen(TFT_CS, TFT_DC, TFT_RST, TOUCH_CS);

void setup() {
  Serial.begin(9600);
  Serial.println("Dispenser Starting..."); 

  screen.begin();
}


void loop() {
  screen.update();
}

