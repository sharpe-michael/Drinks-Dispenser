#include <Arduino.h>
#include "screen/ScreenController.h"

// For the Adafruit shield, these are the default.
#define TFT_DC 4
#define TFT_CS 2
#define TFT_RST 22
#define TOUCH_CS 15

ScreenController screen(TFT_CS, TFT_DC, TFT_RST, TOUCH_CS);

void setup() {
  Serial.begin(9600);
  Serial.println("Dispenser Starting..."); 

  screen.begin();
}


void loop() {
  screen.update();
}

