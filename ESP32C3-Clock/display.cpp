#include <time.h>
#include "Arduino.h"
#include "freertos/FreeRTOS.h"
#include "freertos/semphr.h"
#include "display.h"
#include "app_state.h"
#include "display_constants.h"
#include "timing_constants.h"
#include "pins.h"
#include "config.h"

#if SCREENSHOT_MODE
  CapturableTFT TFT_display(PIN_RST, PIN_DC, PIN_CS);
#else
  DIYables_TFT_GC9A01_Round TFT_display(PIN_RST, PIN_DC, PIN_CS);
#endif

static SemaphoreHandle_t displayMutex = NULL;
static ClockFace* activeFace = NULL;

bool getDisplayTime(struct tm* timeinfo) {
  #if SCREENSHOT_MODE
    *timeinfo = {};
    timeinfo->tm_year = SCREENSHOT_YEAR - 1900;
    timeinfo->tm_mon  = SCREENSHOT_MONTH - 1;
    timeinfo->tm_mday = SCREENSHOT_DAY;
    timeinfo->tm_hour = SCREENSHOT_HOUR;
    timeinfo->tm_min  = SCREENSHOT_MIN;
    timeinfo->tm_sec  = 0;
    mktime(timeinfo);
    return true;
  #else
    return getLocalTime(timeinfo);
  #endif
}

#if SCREENSHOT_MODE
  void screenshotCaptureStrip(int stripY0, uint16_t* outBuffer) {
    struct tm timeinfo;
    TFT_display.beginCapture(stripY0);
    if (activeFace != NULL && getDisplayTime(&timeinfo)) {
      activeFace->reset();
      activeFace->draw(CONNECTED_SYNCED, false, timeinfo);
    }
    TFT_display.endCapture();
    memcpy(outBuffer, TFT_display.stripBuffer(), SCREEN_WIDTH * CAPTURE_STRIP_HEIGHT * sizeof(uint16_t));
  }
#endif

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

  struct tm timeinfo;
  static bool blinkState = false;
  static unsigned long lastBlink = 0;
  static AppState lastState = NOT_CONFIGURED;

  unsigned long now = millis();
  if (now - lastBlink >= BLINK_INTERVAL_MS) {
    blinkState = !blinkState;
    lastBlink = now;
  }

  AppState state = getAppState();

  if (state == RESET_PENDING) {
    if (lastState != state) {
      displayResetQuestion();
    }
    lastState = state;
    return;
  }

  if (state == NOT_CONFIGURED) {
    displayWifiSetupInstructions();
    lastState = state;
    return;
  }

  if (lastState == RESET_PENDING || lastState == NOT_CONFIGURED) {
    activeFace->reset();
  }

  lastState = state;
  if (getDisplayTime(&timeinfo)) {
    activeFace->draw(state, blinkState, timeinfo);
  }
}

void displayWifiError() {
  TFT_display.fillScreen(COLOR_BACKGROUND);
  TFT_display.setTextColor(COLOR_RED, COLOR_BACKGROUND);
  TFT_display.setTextSize(2);

  static char str1[] = "WiFi Failed";
  static char str2[] = "Restarting...";

  TFT_display.setCursor((SCREEN_WIDTH - (strlen(str1) * 12)) / 2, CENTER_Y - 20);
  TFT_display.print(str1);
  TFT_display.setTextSize(2);
  TFT_display.setCursor((SCREEN_WIDTH - (strlen(str2) * 12)) / 2, CENTER_Y + 15);
  TFT_display.print(str2);
}

void displayResetQuestion() {
  TFT_display.fillScreen(COLOR_BACKGROUND);
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
