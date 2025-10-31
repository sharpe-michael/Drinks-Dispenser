#include "SPI.h"
#include "Adafruit_GFX.h"
#include "Adafruit_ILI9341.h"
#include "ScreenController.h"
#include <XPT2046_Touchscreen.h>

#define DEBUG_TOUCH false

ScreenController::ScreenController(int8_t tftCsPin, int8_t dcPin, int8_t rstPin, int8_t touchCSPin, LEDController* ledCtrl, PumpController* pump1, PumpController* pump2, PumpController* pump3, ServoController* servoCtrl)
    : tft(Adafruit_ILI9341(tftCsPin, dcPin, rstPin)), ts(touchCSPin)
{
    this->ledController = ledCtrl;
    this->pump1 = pump1;
    this->pump2 = pump2;
    this->pump3 = pump3;
    this->servoController = servoCtrl;

    pinMode(tftCsPin, OUTPUT);
    digitalWrite(tftCsPin, HIGH); // Deselect display
    pinMode(touchCSPin, OUTPUT);
    digitalWrite(touchCSPin, HIGH); // Deselect touch

    // Initialize Regular Menu Buttons
    // Regular Menu Buttons - "Sezier" look: rounded corners, bold colors, larger text
    regularMenuButtons[0] = {20, 30, 100, 50, "Drink 1", ILI9341_WHITE, ILI9341_BLUE};
    regularMenuButtons[2] = {20, 100, 210, 50, "Drink 2...", ILI9341_WHITE, ILI9341_RED};
    regularMenuButtons[1] = {130, 30, 100, 50, "Test", ILI9341_WHITE, ILI9341_GREEN};

    // Test Menu Buttons - "Sezier" look: more vibrant backgrounds, bigger, rounded
    testMenuButtons[0] = {20, 30, 70, 50, "P1", ILI9341_WHITE, ILI9341_CYAN};
    testMenuButtons[1] = {100, 30, 70, 50, "P2", ILI9341_WHITE, ILI9341_MAGENTA};
    testMenuButtons[2] = {180, 30, 70, 50, "P3", ILI9341_WHITE, ILI9341_YELLOW};
    testMenuButtons[3] = {20, 90, 70, 50, "Servo", ILI9341_WHITE, ILI9341_ORANGE};
    testMenuButtons[4] = {100, 90, 70, 50, "LEDS", ILI9341_WHITE, ILI9341_PURPLE};
    testMenuButtons[5] = {20, 150, 230, 50, "Back...", ILI9341_WHITE, ILI9341_RED};
}

// Animation state variables
int16_t eye_x = 80;
int16_t eye_y = 80;
int8_t eye_dx = 3;
int8_t eye_dy = 2;
bool isBlinking = false;
unsigned long blinkStartTime = 0;
unsigned long nextBlinkTime = 0;

// Store previous eye position
int16_t prev_eye_x = eye_x;
int16_t prev_eye_y = eye_y;

ScreenState screenState = IDLE;
ScreenState lastScreenState = IDLE;

// Calibration values (adjust for your hardware)
#define TS_MINX 200
#define TS_MAXX 3800
#define TS_MINY 200
#define TS_MAXY 3800

// Map raw touch to pixel coordinates (member functions)
int16_t ScreenController::mapTouchX(int16_t rawX)
{
    return constrain(map(rawX, TS_MINX, TS_MAXX, 0, tft.width() - 1), 0, tft.width() - 1);
}
int16_t ScreenController::mapTouchY(int16_t rawY)
{
    return constrain(map(rawY, TS_MINY, TS_MAXY, 0, tft.height() - 1), 0, tft.height() - 1);
}

void ScreenController::begin()
{
    tft.begin();
    ts.begin();
    ts.setRotation(1);  // Match screen orientation
    tft.setRotation(3); // Landscape mode
    nextUpdateTime = millis();
    nextBlinkTime = millis() + random(2000, 5000); // Blink every 2-5 seconds
    tft.fillScreen(ILI9341_BLACK);
    screenState = IDLE;

    // Set initial active menu
    activeMenuButtons = regularMenuButtons;
    numActiveMenuButtons = REGULAR_BUTTON_COUNT;
}

void ScreenController::drawCircle(int16_t x0, int16_t y0, int16_t r, uint16_t color)
{
    tft.drawCircle(x0, y0, r, color);
}

void ScreenController::showRegularMenu()
{
    tft.fillScreen(ILI9341_BLACK);

    for (int i = 0; i < REGULAR_BUTTON_COUNT; ++i)
    {
        Button &btn = regularMenuButtons[i];
        drawButton(btn.x, btn.y, btn.w, btn.h, btn.label, btn.color, btn.bg, true);
    }
}

void ScreenController::showTestMenu()
{
    tft.fillScreen(ILI9341_BLACK);

    for (int i = 0; i < TEST_BUTTON_COUNT; ++i)
    {
        Button &btn = testMenuButtons[i];
        drawButton(btn.x, btn.y, btn.w, btn.h, btn.label, btn.color, btn.bg, true);
    }
}

void ScreenController::showMenu()
{
    tft.fillScreen(ILI9341_BLACK);
    if (currentMenu == REGULAR)
    {
        showRegularMenu();
    }
    else
    {
        showTestMenu();
    }
}

void ScreenController::update()
{
    unsigned long now = millis();
    if (screenState != lastScreenState)
    {
        tft.fillScreen(ILI9341_BLACK);
        if (screenState == ACTIVE)
        {
            showMenu();
        }
        lastScreenState = screenState;
    }
    if (screenState == IDLE)
    {
        if (ts.touched())
        {
            screenState = ACTIVE;
            return;
        }
        // Move eye
        if (now >= nextUpdateTime)
        {
            // Erase previous eye by drawing over it with background color
            drawEye(prev_eye_x, prev_eye_y, false, ILI9341_BLACK);
            prev_eye_x = eye_x;
            prev_eye_y = eye_y;
            eye_x += eye_dx;
            eye_y += eye_dy;
            if (eye_x < 60 || eye_x > tft.width() - 60)
                eye_dx = -eye_dx;
            if (eye_y < 60 || eye_y > tft.height() - 60)
                eye_dy = -eye_dy;
            drawEye(eye_x, eye_y, isBlinking, ILI9341_WHITE);
            nextUpdateTime = now + 100; // Smooth movement
        }
        // Handle blinking
        if (!isBlinking && now >= nextBlinkTime)
        {
            isBlinking = true;
            blinkStartTime = now;
            drawEye(eye_x, eye_y, true, ILI9341_WHITE);
        }
        if (isBlinking && now - blinkStartTime > 200)
        { // Blink lasts 200ms
            isBlinking = false;
            nextBlinkTime = now + random(2000, 5000);
            drawEye(eye_x, eye_y, false, ILI9341_WHITE);
        }
    }
    else if (screenState == ACTIVE)
    {
        if (ts.touched())
        {
            TS_Point p = ts.getPoint();
            // Map touch coordinates to pixels
            int16_t tx = mapTouchX(p.x);
            int16_t ty = mapTouchY(p.y);
            if (p.z < 1200)
            {
                return; // Ignore light touches
            }
            if (DEBUG_TOUCH)
            {
                Serial.print("Touch coords: X=");
                Serial.print(tx);
                Serial.print(", Y=");
                Serial.print(ty);
                Serial.print(" Pressure=");
                Serial.println(p.z);
                // Draw debug circle at every touch
                tft.drawCircle(tx, ty, 10, ILI9341_RED);
            }
            lastTouch.x = tx;
            lastTouch.y = ty;
            Button *menuButtons = (currentMenu == REGULAR) ? regularMenuButtons : testMenuButtons;
            int numMenuButtons = (currentMenu == REGULAR) ? REGULAR_BUTTON_COUNT : TEST_BUTTON_COUNT;
            for (int i = 0; i < numMenuButtons; ++i)
            {
                Button &btn = menuButtons[i];
                if (tx >= btn.x && tx < btn.x + btn.w && ty >= btn.y && ty < btn.y + btn.h)
                {
                    // Button pressed
                    Serial.print("Button pressed: ");
                    Serial.println(btn.label);
                    handleButtonPress(i);
                }
            }
        }
    }
}

void ScreenController::drawEye(int16_t x, int16_t y, bool blink, uint16_t bg)
{
    tft.fillCircle(x, y, 60, bg); // Erase or draw white part
    if (blink)
    {
        tft.fillRect(x - 60, y - 10, 120, 20, ILI9341_BLACK);
    }
    else if (bg == ILI9341_WHITE)
    {
        tft.fillCircle(x, y, 30, ILI9341_BLACK); // Pupil
    }
}

void ScreenController::drawButton(int16_t x, int16_t y, int16_t w, int16_t h, const char *label, uint16_t color, uint16_t bg, bool hasBorder)
{
    tft.fillRect(x, y, w, h, bg);            // Button background
    tft.drawRect(x, y, w, h, ILI9341_WHITE); // Button border
    tft.setTextColor(color);
    tft.setTextSize(2);
    int16_t textX = x + (w - (strlen(label) * 12)) / 2; // Center text
    int16_t textY = y + (h - 16) / 2;
    tft.setCursor(textX, textY);
    tft.print(label);
}

void ScreenController::handleButtonPress(int idx)
{
    if (currentMenu == REGULAR)
    {
        Button &btn = regularMenuButtons[idx];
        if (strcmp(btn.label, "Drink 1") == 0)
        { // Start button
            Serial.println("Dispensing drink 1...");
            // Implement start functionality here
        }
        else if (strcmp(btn.label, "Drink 2...") == 0)
        { // Back button
            Serial.println("Dispensing drink 2...");
            // Implement drink 2 functionality here
        }
        else if (strcmp(btn.label, "Test") == 0)
        { // Test button
            currentMenu = TEST;
            showMenu();
        }
    }
    else if (currentMenu == TEST)
    {
        Button &btn = testMenuButtons[idx];
        if (strcmp(btn.label, "Back...") == 0)
        { // T1 button
            Serial.println("Back to Regular Menu");
            currentMenu = REGULAR;
            showMenu();
        }
        else if (strcmp(btn.label, "P1") == 0)
        { // T1 button
            Serial.println("Pump 1 test");
            this->pump1->dispenseVolume(200); // Dispense 200 mL for testing
        }
        else if (strcmp(btn.label, "P2") == 0)
        { // T2 button
            Serial.println("Pump 2 selected");
            this->pump2->dispenseVolume(200); // Dispense 200 mL for testing
        }
        else if (strcmp(btn.label, "P3") == 0)
        { // T3 button
            Serial.println("Pump 3 selected");
            this->pump3->dispenseVolume(200); // Dispense 200 mL for testing
        }
        else if (strcmp(btn.label, "Servo") == 0)
        { // Servo button
            Serial.println("Servo test selected");
            this->servoController->close();
            delay(1000);    
            this->servoController->open();            
        }
        else if (strcmp(btn.label, "LEDS") == 0)
        { // LEDS button
            Serial.println("LEDs test selected");
            LEDController* ledCtrl = this->ledController;
            if (ledCtrl) {
                if (ledCtrl->mode == DISPENSING_LEDS) {
                    ledCtrl->mode = FINISHED_LEDS;
                } else if (ledCtrl->mode == FINISHED_LEDS) {
                    ledCtrl->mode = IDLE_LEDS;
                } else
                {
                    ledCtrl->mode = DISPENSING_LEDS;
                }
            }
        };
    }
}