#include <WiFi.h>
#include <math.h>
#include <time.h>
#include "Arduino.h"
#include "freertos/FreeRTOS.h"
#include "freertos/semphr.h"
#include "display.h"
#include "display_constants.h"
#include "pins.h"
#include "icons.h"
#include "config.h"
#include "app_state.h"

DIYables_TFT_GC9A01_Round TFT_display(PIN_RST, PIN_DC, PIN_CS);

static SemaphoreHandle_t displayMutex = NULL;
static ClockFace* activeFace = NULL;

void takeDisplayMutex() {
  if (displayMutex != NULL) {
    xSemaphoreTake(displayMutex, portMAX_DELAY);
  }
}

void giveDisplayMutex() {
  if (displayMutex != NULL) {
    xSemaphoreGive(displayMutex);
  }
}

void displaySetup() {
  displayMutex = xSemaphoreCreateMutex();
  Serial.println("Display mutex created.");
}

void setClockFace(ClockFace* face) {
  activeFace = face;
}

void redrawDisplay() {
  if (activeFace == NULL) {
    return;
  }

  static bool blinkState = false;
  static unsigned long lastBlink = 0;
  unsigned long now = millis();
  if (now - lastBlink >= BLINK_INTERVAL_MS) {
    blinkState = !blinkState;
    lastBlink = now;
  }

  activeFace->draw(blinkState);
}

void displayResetQuestion() {
  TFT_display.setTextColor(COLOR_YELLOW, COLOR_BACKGROUND);
  TFT_display.setTextSize(3);

  static char str1[] = "Reset";
  static char str2[] = "config?";
  static char str3[] = "Double click";
  static char str4[] = "to confirm";

  TFT_display.setCursor((SCREEN_WIDTH - (strlen(str1) * 18)) / 2, CENTER_Y - 60);
  TFT_display.print(str1);
  TFT_display.setCursor((SCREEN_WIDTH - (strlen(str2) * 18)) / 2, CENTER_Y - 30);
  TFT_display.print(str2);

  TFT_display.setTextSize(2);
  TFT_display.setCursor((SCREEN_WIDTH - (strlen(str3) * 12)) / 2, CENTER_Y + 12);
  TFT_display.print(str3);
  TFT_display.setCursor((SCREEN_WIDTH - (strlen(str4) * 12)) / 2, CENTER_Y + 40);
  TFT_display.print(str4);
}

void displayWifiSetupInstructions() {
  TFT_display.fillScreen(COLOR_BACKGROUND);
  TFT_display.setTextColor(COLOR_YELLOW, COLOR_BACKGROUND);

  static char str1[] = "Connect to";
  static char str2[] = "WiFi hotspot:";
  char ssidLine[40];
  char pwLine[40];
  snprintf(ssidLine, sizeof(ssidLine), "SSID: %s", WIFI_HOTSPOT_SSID);
  snprintf(pwLine,   sizeof(pwLine),   "PW: %s",   WIFI_HOTSPOT_PASSWORD);

  TFT_display.setTextSize(2);
  TFT_display.setCursor((SCREEN_WIDTH - (strlen(str1) * 12)) / 2, CENTER_Y - 60);
  TFT_display.print(str1);
  TFT_display.setCursor((SCREEN_WIDTH - (strlen(str2) * 12)) / 2, CENTER_Y - 35);
  TFT_display.print(str2);

  TFT_display.setTextColor(COLOR_CLOCKFACE, COLOR_BACKGROUND);
  TFT_display.setCursor((SCREEN_WIDTH - (strlen(ssidLine) * 12)) / 2, CENTER_Y);
  TFT_display.print(ssidLine);
  TFT_display.setCursor((SCREEN_WIDTH - (strlen(pwLine) * 12)) / 2, CENTER_Y + 25);
  TFT_display.print(pwLine);
}

static void drawIcon(bool visible, int x, const uint16_t* icon) {
  if (visible) {
    TFT_display.drawRGBBitmap(x, 45, icon, ICON_WIDTH, ICON_HEIGHT);
  }
  else {
    TFT_display.fillRect(x, 45, ICON_WIDTH, ICON_HEIGHT, COLOR_BACKGROUND);
  }
}
