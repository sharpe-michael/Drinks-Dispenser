#ifndef SCREENCONTROLLER_H
#define SCREENCONTROLLER_H

#include "SPI.h"
#include "Adafruit_GFX.h"
#include "Adafruit_ILI9341.h"
#include <XPT2046_Touchscreen.h>

enum ScreenState { IDLE, ACTIVE };

class ScreenController {
public:
    ScreenController(int8_t screenCSPin, int8_t dcPin, int8_t rstPin, int8_t touchCSPin);
    void begin();
    void update();
private:
    Adafruit_ILI9341 tft;
    XPT2046_Touchscreen ts;
    unsigned long nextUpdateTime;
    int16_t eye_x = 80;
    int16_t eye_y = 80;
    int8_t eye_dx = 3;
    int8_t eye_dy = 2;
    bool isBlinking = false;
    unsigned long blinkStartTime = 0;
    unsigned long nextBlinkTime = 0;
    int16_t prev_eye_x = eye_x;
    int16_t prev_eye_y = eye_y;
    ScreenState screenState = ACTIVE;
    ScreenState lastScreenState = IDLE;

    struct Button {
        int16_t x, y, w, h;
        const char* label;
        uint16_t color, bg;
    };
    static const int numMenuButtons = 6;
    Button menuButtons[numMenuButtons];

    void drawText(int16_t x, int16_t y, const char* text, uint16_t color, uint8_t size);
    void showRegularMenu();
    void drawCircle(int16_t x0, int16_t y0, int16_t r, uint16_t color);
    void drawEye(int16_t x, int16_t y, bool blink, uint16_t bg);
    void showMenu();
    void drawButton(int16_t x, int16_t y, int16_t w, int16_t h, const char* label, uint16_t color, uint16_t bg, bool hasBorder);
    void handleButtonPress(int idx);
};

#endif // SCREENCONTROLLER_H