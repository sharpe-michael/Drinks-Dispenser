#include "SPI.h"
#include "Adafruit_GFX.h"
#include "Adafruit_ILI9341.h"
#include "ScreenController.h"
#include <XPT2046_Touchscreen.h>

ScreenController::ScreenController(int8_t tftCsPin, int8_t dcPin, int8_t rstPin, int8_t touchCSPin)
    : tft(Adafruit_ILI9341(tftCsPin, dcPin, rstPin)), ts(touchCSPin) {
    pinMode(tftCsPin, OUTPUT);
    digitalWrite(tftCsPin, HIGH); // Deselect display
    pinMode(touchCSPin, OUTPUT);
    digitalWrite(touchCSPin, HIGH); // Deselect touch
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


ScreenState screenState = ACTIVE;
ScreenState lastScreenState = IDLE;

// Button structure for easy hit testing
struct Button {
    int16_t x, y, w, h;
    const char* label;
    uint16_t color, bg;
};
Button menuButtons[] = {
    {0, 0, 320/3, 120, "Pump 1", ILI9341_WHITE, ILI9341_NAVY},
    {320/3, 0, 320/3, 120, "Pump 2", ILI9341_WHITE, ILI9341_DARKGREEN},
    {2*320/3, 0, 320/3, 120, "Pump 3", ILI9341_WHITE, ILI9341_RED},
    {0, 120, 320/3, 120, "Servo", ILI9341_BLACK, ILI9341_ORANGE},
    {320/3, 120, 320/3, 120, "LEDs", ILI9341_BLACK, ILI9341_PINK},
    {2*320/3, 120, 320/3, 120, "Back...", ILI9341_WHITE, ILI9341_BLACK}
};
const int numMenuButtons = sizeof(menuButtons)/sizeof(menuButtons[0]);

enum MenuType { REGULAR, TEST };
MenuType currentMenu = TEST;

void ScreenController::begin() {
    tft.begin();
    ts.begin();
    ts.setRotation(1); // Match screen orientation
    tft.setRotation(3); // Landscape mode
    nextUpdateTime = millis();
    nextBlinkTime = millis() + random(2000, 5000); // Blink every 2-5 seconds
    tft.fillScreen(ILI9341_BLACK);
}

void ScreenController::drawCircle(int16_t x0, int16_t y0, int16_t r, uint16_t color) {
    tft.drawCircle(x0, y0, r, color);
}

void ScreenController::showRegularMenu() {
    tft.fillScreen(ILI9341_BLACK);
    // Example regular menu layout
    drawButton(0, 200, 320, 40, "Test Menu", ILI9341_WHITE, ILI9341_BLACK, true);
    drawButton(0, 0, 160, 200, "Drink 1", ILI9341_WHITE, ILI9341_BLUE, true);
    drawButton(160, 0, 160, 200, "Drink 2", ILI9341_WHITE, ILI9341_GREEN, true);
}

void ScreenController::showMenu() {
    tft.fillScreen(ILI9341_BLACK);
    if (currentMenu == REGULAR) {
        showRegularMenu();
    } else {
        // Test Menu
        drawButton(0, 0, 320/3, 120, "Pump 1", ILI9341_WHITE, ILI9341_NAVY, true);
        drawButton(320/3, 0, 320/3, 120, "Pump 2", ILI9341_WHITE, ILI9341_DARKGREEN, true);
        drawButton(2*320/3, 0, 320/3, 120, "Pump 3", ILI9341_WHITE, ILI9341_RED, true);
        drawButton(0, 120, 320/3, 120, "Servo", ILI9341_BLACK, ILI9341_ORANGE, true);
        drawButton(320/3, 120, 320/3, 120, "LEDs", ILI9341_BLACK, ILI9341_PINK, true);
        drawButton(2*320/3, 120, 320/3, 120, "Back...", ILI9341_WHITE, ILI9341_BLACK, true);   
    }
}

void ScreenController::update() {
    unsigned long now = millis();
    if (currentMenu == TEST && millis() > 10000) {
        
    }
    if (screenState != lastScreenState) {
        tft.fillScreen(ILI9341_BLACK);
        if (screenState == ACTIVE) {
            showMenu();
        }
        lastScreenState = screenState;
    }
    if (screenState == IDLE) {
        // Move eye
        if (now >= nextUpdateTime) {
            // Erase previous eye by drawing over it with background color
            drawEye(prev_eye_x, prev_eye_y, false, ILI9341_BLACK);
            prev_eye_x = eye_x;
            prev_eye_y = eye_y;
            eye_x += eye_dx;
            eye_y += eye_dy;
            if (eye_x < 60 || eye_x > tft.width() - 60) eye_dx = -eye_dx;
            if (eye_y < 60 || eye_y > tft.height() - 60) eye_dy = -eye_dy;
            drawEye(eye_x, eye_y, isBlinking, ILI9341_WHITE);
            nextUpdateTime = now + 100; // Smooth movement
        }
        // Handle blinking
        if (!isBlinking && now >= nextBlinkTime) {
            isBlinking = true;
            blinkStartTime = now;
            drawEye(eye_x, eye_y, true, ILI9341_WHITE);
        }
        if (isBlinking && now - blinkStartTime > 200) { // Blink lasts 200ms
            isBlinking = false;
            nextBlinkTime = now + random(2000, 5000);
            drawEye(eye_x, eye_y, false, ILI9341_WHITE);
        }
    } else if (screenState == ACTIVE) {
        // Menu is only drawn once when entering ACTIVE
        // Add menu interaction logic here
        // Touch handling for menu
        if (ts.touched()) {
            TS_Point p = ts.getPoint();
            // Map touch coordinates if needed (depends on wiring)
            int16_t tx = p.x;
            int16_t ty = p.y;
            // If needed, swap/flip tx/ty here
            for (int i = 0; i < numMenuButtons; ++i) {
                Button& btn = menuButtons[i];
                if (tx >= btn.x && tx < btn.x + btn.w && ty >= btn.y && ty < btn.y + btn.h) {
                    // Button pressed
                    handleButtonPress(i);
                }
            }
        }
    }
}

void ScreenController::drawEye(int16_t x, int16_t y, bool blink, uint16_t bg) {
    tft.fillCircle(x, y, 60, bg); // Erase or draw white part
    if (blink) {
        tft.fillRect(x - 60, y - 10, 120, 20, ILI9341_BLACK);
    } else if (bg == ILI9341_WHITE) {
        tft.fillCircle(x, y, 30, ILI9341_BLACK); // Pupil
    }
}

void ScreenController::drawButton(int16_t x, int16_t y, int16_t w, int16_t h, const char* label, uint16_t color, uint16_t bg, bool hasBorder ) {
    tft.fillRect(x, y, w, h, bg); // Button background
    tft.drawRect(x, y, w, h, ILI9341_WHITE); // Button border
    tft.setTextColor(color);
    tft.setTextSize(2);
    int16_t textX = x + (w - (strlen(label) * 12)) / 2; // Center text
    int16_t textY = y + (h - 16) / 2;
    tft.setCursor(textX, textY);
    tft.print(label);
}

void ScreenController::handleButtonPress(int idx) {
    if (currentMenu == TEST && strcmp(menuButtons[idx].label, "Back...") == 0) {
        currentMenu = REGULAR;
        showMenu();
        return;
    }
    // Example actions
    if (strcmp(menuButtons[idx].label, "Back...") == 0) {
        screenState = IDLE;
    } else {
        // Add your own actions for each button
        tft.fillScreen(ILI9341_BLACK);
        tft.setTextColor(ILI9341_YELLOW);
        tft.setTextSize(3);
        tft.setCursor(40, 100);
        tft.print(menuButtons[idx].label);
        delay(500); // Show feedback (non-blocking recommended for production)
        showMenu();
    }
}