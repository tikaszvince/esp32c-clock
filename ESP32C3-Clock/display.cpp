#include <time.h>
#include "Arduino.h"
#include "freertos/FreeRTOS.h"
#include "freertos/semphr.h"
#include <math.h>
#include "display.h"
#include "app_state.h"
#include "display_constants.h"
#include "timing_constants.h"
#include "pins.h"
#include "config.h"
#if ENCODER_ENABLED
  #include "face_manager.h"
#endif

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

#if ENCODER_ENABLED
  static void drawGracePeriodOverlay(float fraction) {
    // Thin arc at the outer edge, draining clockwise from the top.
    static const int ARC_RADIUS = 117;
    static const float ARC_STEP_DEG = 0.5f;
    static const uint16_t ARC_COLOR = DIYables_TFT::colorRGB(0, 220, 255);

    float filledDeg = fraction * 360.0f;
    for (float angle = 0.0f; angle < 360.0f; angle += ARC_STEP_DEG) {
      float rad = (angle - 90.0f) * PI / 180.0f;
      int x = CENTER_X + (int)roundf(ARC_RADIUS * cosf(rad));
      int y = CENTER_Y + (int)roundf(ARC_RADIUS * sinf(rad));
      uint16_t color = (angle < filledDeg) ? ARC_COLOR : COLOR_BACKGROUND;
      TFT_display.drawPixel(x, y, color);
    }

    // "Click to save" text centered near the bottom.
    static const char* label = "Click to save";
    TFT_display.setTextSize(1);
    TFT_display.setTextColor(ARC_COLOR, COLOR_BACKGROUND);
    int textX = (SCREEN_WIDTH - strlen(label) * 6) / 2;
    TFT_display.setCursor(textX, SCREEN_HEIGHT - 20);
    TFT_display.print(label);
  }
#endif

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
    #if ENCODER_ENABLED
      if (
        faceManagerIsGracePeriodActive()
        && !activeFace->handlesGracePeriodOverlay()
      ) {
        drawGracePeriodOverlay(faceManagerGetGracePeriodFraction());
      }
    #endif
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
