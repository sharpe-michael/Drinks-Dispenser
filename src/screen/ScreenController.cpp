#include "SPI.h"
#include "Adafruit_GFX.h"
#include "Adafruit_ILI9341.h"
#include "ScreenController.h"
#include <XPT2046_Touchscreen.h>

#define DEBUG_TOUCH true

ScreenController::ScreenController(int8_t tftCsPin, int8_t dcPin, int8_t rstPin, int8_t touchCSPin, LEDController *ledCtrl, PumpController *pump1, PumpController *pump2, ServoController *servoCtrl)
    : tft(Adafruit_ILI9341(tftCsPin, dcPin, rstPin)), ts(touchCSPin)
{
    this->ledController = ledCtrl;
    this->pump1 = pump1;
    this->pump2 = pump2;
    // this->pump3 = pump3;
    this->servoController = servoCtrl;

    pinMode(tftCsPin, OUTPUT);
    digitalWrite(tftCsPin, HIGH); // Deselect display
    pinMode(touchCSPin, OUTPUT);
    digitalWrite(touchCSPin, HIGH); // Deselect touch

    // Initialize Regular Menu Buttons
    regularMenuButtons[0] = {0, 0, 170, 200, "Vod (Dbl)+Cran", ILI9341_WHITE, ILI9341_RED};
    regularMenuButtons[1] = {170, 0, 170, 200, "Vod (Dbl)", ILI9341_BLUE, ILI9341_WHITE};
    regularMenuButtons[2] = {0, 200, 340, 40, "Test", ILI9341_WHITE, ILI9341_GREEN};

    // Test Menu Buttons
    testMenuButtons[0] = {20, 30, 70, 50, "P1", ILI9341_WHITE, ILI9341_CYAN};
    testMenuButtons[1] = {100, 30, 70, 50, "P2", ILI9341_WHITE, ILI9341_MAGENTA};
    testMenuButtons[2] = {20, 90, 70, 50, "Servo", ILI9341_WHITE, ILI9341_ORANGE};
    testMenuButtons[3] = {100, 90, 70, 50, "LEDS", ILI9341_WHITE, ILI9341_PURPLE};
    testMenuButtons[4] = {20, 150, 230, 50, "Back...", ILI9341_WHITE, ILI9341_RED};
}

// Animation state variables
int16_t eye_x = 80;
int16_t eye_y = 80;
int8_t eye_dx = 3;
int8_t eye_dy = 2;
bool isBlinking = false;
uint32_t blinkStartTime = 0;
uint32_t nextBlinkTime = 0;

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
    // uint8_t x = tft.readcommand8(ILI9341_RDMODE);
    // if (x != 0x05)
    // {
    //     begin();
    // }
    if (nextScreenStateTime != 0 && millis() >= nextScreenStateTime)
    {
        screenState = nextScreenStateValue;
        nextScreenStateTime = 0;
    }
    if (screenState != lastScreenState)
    {
        tft.fillScreen(ILI9341_BLACK);
        if (screenState == IDLE)
        {
            this->ledController->mode = IDLE_LEDS;
        }
        if (screenState == ACTIVE)
        {
            showMenu();
        }
        if (screenState == FINISHED)
        {
            this->nextScreenStateValue = IDLE;
            this->nextScreenStateTime = millis() + 5000; // After 15
            this->servoController->open();
        }
        if (lastScreenState == FINISHED)
        {
            begin(); // Reset to initial state
        }
        lastScreenState = screenState;
    }
    if (screenState == IDLE)
    {
        if (ts.touched() && !isPhantomTouch(ts.getPoint().x, ts.getPoint().y, ts.getPoint().z))
        {
            screenState = ACTIVE;
            return;
        }
        // Move eye
        if (millis() >= nextUpdateTime)
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
            nextUpdateTime = millis() + 100; // Smooth movement
        }
        // Handle blinking
        if (!isBlinking && millis() >= nextBlinkTime)
        {
            isBlinking = true;
            blinkStartTime = millis();
            drawEye(eye_x, eye_y, true, ILI9341_WHITE);
        }
        if (isBlinking && millis() - blinkStartTime > 200)
        { // Blink lasts 200ms
            isBlinking = false;
            nextBlinkTime = millis() + random(2000, 5000);
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

            if (isPhantomTouch(tx, ty, p.z))
            {
                // Ignore phantom touch
                return;
            }

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

        uint32_t durationSinceLastGoodTouch = millis() - lastGoodTouchTime;
        if (durationSinceLastGoodTouch > 5000)
        {
            Serial.println("No touch detected for 5 seconds, returning to IDLE mode. Last good touch time: " + String(lastGoodTouchTime) + ", now: " + String(millis()) + ", duration: " + String(durationSinceLastGoodTouch) + "ms");
            // No good touch for 10 seconds, go back to IDLE
            screenState = IDLE;
            tft.fillScreen(ILI9341_BLACK);
            // Reset eye position
            eye_x = tft.width() / 2;
            eye_y = tft.height() / 2;
            prev_eye_x = eye_x;
            prev_eye_y = eye_y;
            eye_dx = 3;
            eye_dy = 2;
            isBlinking = false;
            nextBlinkTime = millis() + random(2000, 5000);
            return;
        }
    }
    else if (screenState == DISPENSING)
    {
        tft.fillScreen(ILI9341_BLACK);
        tft.setTextColor(ILI9341_WHITE);
        tft.setTextSize(3);
        tft.setCursor(60, tft.height() / 2 - 10);
        tft.print("Dispensing...");
    }
    else if (screenState == FINISHED)
    {
        tft.fillScreen(ILI9341_BLACK);
        tft.setTextColor(ILI9341_GREEN);
        tft.setTextSize(3);
        tft.setCursor(80, tft.height() / 2 - 10);
        tft.print("Finished!");
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
        if (strcmp(btn.label, "Vod (Dbl)+Cran") == 0)
        { // Start button
            Serial.println("Dispensing drink 1...");
            screenState = DISPENSING;
            this->servoController->close();
            this->ledController->mode = DISPENSING_LEDS;

            uint32_t runTimePump2 = this->pump2->dispenseVolume(50); // Dispense 100 mL for drink 1 after 2 seconds
            uint32_t runTimePump1 = this->pump1->dispenseVolume(150, runTimePump2 + 300);               // Dispense 250 mL for drink 1
            // uint32_t runTimePump3 = this->pump3->dispenseVolume(50, runTimePump1 + 300 + runTimePump2 + 300);  // Dispense 50 mL for drink 1 after 4 seconds

            uint32_t totalDispenseTime = runTimePump1 + runTimePump2 + 300; // + runTimePump3 + 600; // Total time including delays

            this->ledController->nextUpdateMode = FINISHED_LEDS;
            this->nextScreenStateValue = FINISHED;
            this->ledController->nextUpdateTime = millis() + totalDispenseTime; // After dispense seconds, switch mode
            this->nextScreenStateTime = millis() + totalDispenseTime;           // After dispense seconds
        }
        else if (strcmp(btn.label, "Vod (Dbl)") == 0)
        { // Start button
            Serial.println("Dispensing drink 2...");
            screenState = DISPENSING;
            this->servoController->close();
            this->ledController->mode = DISPENSING_LEDS;

            uint32_t runTimePump2 = this->pump2->dispenseVolume(50); // Dispense 50 mL of Vodka

            uint32_t totalDispenseTime = runTimePump2; // + runTimePump3 + 600; // Total time including delays

            this->ledController->nextUpdateMode = FINISHED_LEDS;
            this->nextScreenStateValue = FINISHED;
            this->ledController->nextUpdateTime = millis() + totalDispenseTime; // After dispense seconds, switch mode
            this->nextScreenStateTime = millis() + totalDispenseTime;           // After dispense seconds
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
            Serial.println("Pump 1 selected");
            screenState = DISPENSING;
            this->servoController->close();
            this->ledController->mode = DISPENSING_LEDS;
            uint32_t runtime = this->pump1->dispenseVolume(50);                       // Dispense 200 mL for testing
            this->ledController->nextUpdateMode = FINISHED_LEDS;
            this->nextScreenStateValue = FINISHED;
            this->ledController->nextUpdateTime = millis() + runtime; // After 10 seconds, switch mode
            this->nextScreenStateTime = millis() + runtime;           // After 10 seconds
        }
        else if (strcmp(btn.label, "P2") == 0)
        { // T2 button
            Serial.println("Pump 2 selected");
            screenState = DISPENSING;
            this->servoController->close();
            this->ledController->mode = DISPENSING_LEDS;
            uint32_t runtime = this->pump2->dispenseVolume(50);                       // Dispense 200 mL for testing
            this->ledController->nextUpdateMode = FINISHED_LEDS;
            this->nextScreenStateValue = FINISHED;
            this->ledController->nextUpdateTime = millis() + runtime; // After 10 seconds, switch mode
            this->nextScreenStateTime = millis() + runtime;           // After 10 seconds
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
            if (this->ledController->mode == DISPENSING_LEDS)
            {
                Serial.println("Switching to FINISHED_LEDS mode");
                this->ledController->mode = FINISHED_LEDS;
            }
            else if (this->ledController->mode == FINISHED_LEDS)
            {
                Serial.println("Switching to IDLE_LEDS mode");
                this->ledController->mode = IDLE_LEDS;
            }
            else
            {
                Serial.println("Switching to DISPENSING_LEDS mode");
                this->ledController->mode = DISPENSING_LEDS;
            }
        };
    }
}

bool ScreenController::isPhantomTouch(int16_t tx, int16_t ty, uint16_t pressure)
{

    // Phantom touch filtering
    /**
     * It's likely to be a phantom touch if:
     * - The touch position is far away from the last touch position
     * - The touch pressure is low
     * - The touch is very brief
     *
     * Therefore, we ignore touches that are:
     * - More than 50 pixels away from last touch
     * - Pressure below 1500
     * - Duration less than 50ms
     */
    if (pressure < 1200)
    {
        return true; // Ignore light touches
    }

    if (pressure > 2400)
    {
        if (DEBUG_TOUCH)
        {
            Serial.println("Ignored touch due to overly hard pressure");
        }
        return true; // Ignore overly hard touches
    }
    if (millis() - this->lastTouchTime > 200)
    {
        if (DEBUG_TOUCH)
        {
            Serial.println("Ignored touch due to brief duration: " + String(millis() - lastTouchTime) + "ms");
        }
        this->lastTouchTime = millis();
        return true; // Ignore very brief touches
    }
    this->lastTouchTime = millis();
    if (lastTouch.x != -1 && lastTouch.y != -1)
    {
        int16_t dx = abs(tx - lastTouch.x);
        int16_t dy = abs(ty - lastTouch.y);
        if (dx > 50 || dy > 50)
        {
            if (DEBUG_TOUCH)
            {
                Serial.println("Ignored touch due to large movement: " + String(dx) + "px, " + String(dy) + "px");
            }
            lastTouch.x = tx;
            lastTouch.y = ty;
            return true; // Ignore touches far from last touch
        }
    }
    lastTouch.x = tx;
    lastTouch.y = ty;

    if (DEBUG_TOUCH)
    {
        Serial.print("Touch coords: X=");
        Serial.print(tx);
        Serial.print(", Y=");
        Serial.print(ty);
        Serial.print(" Pressure=");
        Serial.println(pressure);
        // Draw debug circle at every touch
        tft.drawCircle(tx, ty, 10, ILI9341_RED);
    }
    this->lastGoodTouchTime = millis();
    return false; // Valid touch
}