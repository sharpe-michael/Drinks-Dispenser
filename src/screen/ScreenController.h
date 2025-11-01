#ifndef SCREENCONTROLLER_H
#define SCREENCONTROLLER_H

#include "SPI.h"
#include "Adafruit_GFX.h"
#include "Adafruit_ILI9341.h"
#include <XPT2046_Touchscreen.h>
#include "leds/LEDController.h"
#include "pump/PumpController.h"
#include "servo/ServoController.h"

enum ScreenState
{
    IDLE,
    ACTIVE
};
enum MenuType
{
    REGULAR,
    TEST
};

struct Button
{
    int16_t x, y, w, h;
    const char *label;
    uint16_t color, bg;
};

struct TouchPoint
{
    int16_t x;
    int16_t y;
};

class ScreenController
{
public:
    ScreenController(int8_t screenCSPin, int8_t dcPin, int8_t rstPin, int8_t touchCSPin, LEDController *ledCtrl, PumpController *pump1, PumpController *pump2, PumpController *pump3, ServoController *servoCtrl);
    void begin();
    void update();

private:
    Adafruit_ILI9341 tft;
    XPT2046_Touchscreen ts;
    LEDController *ledController;
    PumpController *pump1;  
    PumpController *pump2;
    PumpController *pump3;
    ServoController *servoController;

    // Animation state variables
    unsigned long nextUpdateTime;
    int16_t eye_x = 80;
    int16_t eye_y = 80;
    int8_t eye_dx = 3;
    int8_t eye_dy = 2;
    bool isBlinking = false;
    unsigned long blinkStartTime = 0;
    unsigned long nextBlinkTime = 0;
    int16_t prev_eye_x = 80;
    int16_t prev_eye_y = 80;

    // Screen state variables
    MenuType currentMenu = REGULAR;
    ScreenState screenState = IDLE; // Start in IDLE mode
    ScreenState lastScreenState = IDLE;

    // --- Menu Management Members ---
    static const int REGULAR_BUTTON_COUNT = 3; // Define array size
    static const int TEST_BUTTON_COUNT = 6;    // Define array size

    // The predefined button arrays
    Button regularMenuButtons[REGULAR_BUTTON_COUNT];
    Button testMenuButtons[TEST_BUTTON_COUNT];

    // Pointers to the currently active menu array and its size - IDK what this does yet
    Button *activeMenuButtons;
    int numActiveMenuButtons;

    TouchPoint lastTouch = {-1, -1};
    uint16_t lastTouchTime = 0;
    uint16_t lastGoodTouchTime = 0;

    // Private methods
    void drawText(int16_t x, int16_t y, const char *text, uint16_t color, uint8_t size);
    void showMenu();
    void showRegularMenu();
    void showTestMenu();
    void drawCircle(int16_t x0, int16_t y0, int16_t r, uint16_t color);
    void drawEye(int16_t x, int16_t y, bool blink, uint16_t bg);
    void drawButton(int16_t x, int16_t y, int16_t w, int16_t h, const char *label, uint16_t color, uint16_t bg, bool hasBorder);
    void handleButtonPress(int idx);
    int16_t mapTouchX(int16_t rawX);
    int16_t mapTouchY(int16_t rawY);

    bool isPhantomTouch(int16_t tx, int16_t ty, uint16_t pressure);
};

#endif // SCREENCONTROLLER_H