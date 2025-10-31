#include <Arduino.h>
#include "screen/ScreenController.h"
#include "leds/LEDController.h"
#include "pump/PumpController.h"
#include "servo/ServoController.h"

// For the Adafruit shield, these are the default.
#define TFT_DC 9
#define TFT_CS 10
#define TFT_RST 8
#define TOUCH_CS 7


PumpController pump1(A1);
PumpController pump2(A2);
PumpController pump3(A3);

ServoController servoController(3);

LEDController ledController;
ScreenController screen(TFT_CS, TFT_DC, TFT_RST, TOUCH_CS, &ledController, &pump1, &pump2, &pump3, &servoController);

void setup() {
  Serial.begin(9600);
  Serial.println("Dispenser Starting..."); 

  ledController.begin();
  screen.begin();
}


void loop() {
  screen.update();
  ledController.update();
  pump1.update();
  pump2.update();
  pump3.update();
  servoController.update();
}

