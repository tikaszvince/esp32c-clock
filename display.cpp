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

static bool blinkState = false;
static unsigned long lastBlink = 0;
static SemaphoreHandle_t displayMutex = NULL;

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

void drawClockFace() {
  // Reset screen.
  TFT_display.fillScreen(COLOR_BACKGROUND);
  // Draw outer circle
  TFT_display.drawCircle(CENTER_X, CENTER_Y, CLOCK_RADIUS, COLOR_CLOCKFACE);
  TFT_display.drawCircle(CENTER_X, CENTER_Y, CLOCK_RADIUS - 1, COLOR_CLOCKFACE);

  // Draw hour markers
  for (int i = 0; i < 12; i++) {
    float angle = i * 30 - 90;
    float angleRad = angle * PI / 180.0;

    // Outer point
    int x1 = CENTER_X + (CLOCK_RADIUS - 5) * cos(angleRad);
    int y1 = CENTER_Y + (CLOCK_RADIUS - 5) * sin(angleRad);

    // Inner point
    int x2 = CENTER_X + (CLOCK_RADIUS - 15) * cos(angleRad);
    int y2 = CENTER_Y + (CLOCK_RADIUS - 15) * sin(angleRad);

    // Draw thicker lines for 12, 3, 6, 9
    if (i % 3 == 0) {
      TFT_display.drawLine(x1,     y1,     x2,     y2,     COLOR_CLOCKFACE);
      TFT_display.drawLine(x1 + 1, y1,     x2 + 1, y2,     COLOR_CLOCKFACE);
      TFT_display.drawLine(x1,     y1 + 1, x2,     y2 + 1, COLOR_CLOCKFACE);
    }
    else {
      TFT_display.drawLine(x1, y1, x2, y2, COLOR_CLOCKFACE);
    }
    // Prevent watchdog
    yield();
  }

  // Draw center dot
  TFT_display.fillCircle(CENTER_X, CENTER_Y, 4, COLOR_CENTER_DOT);
  drawTextBox();
}

void drawTextBox() {
  // Draw display box.
  TFT_display.fillRect(TEXTBOX_X, TEXTBOX_Y, TEXTBOX_WIDTH, TEXTBOX_HEIGHT, COLOR_BACKGROUND);
  TFT_display.drawRect(TEXTBOX_X, TEXTBOX_Y, TEXTBOX_WIDTH, TEXTBOX_HEIGHT, COLOR_CLOCKFACE);
}

void redrawTextBox(const char str[]) {
  drawTextBox();
  writeText(str, false);
}

void writeText(const char str[], bool center) {
  TFT_display.setTextColor(COLOR_YELLOW, COLOR_BACKGROUND);
  TFT_display.setTextSize(2);

  int x = TEXTBOX_X + 5;
  if (center) {
    int textWidth = strlen(str) * 12;  // Approximate width
    x = (SCREEN_WIDTH - textWidth) / 2;
  }

  TFT_display.setCursor(x, 170);
  TFT_display.print(str);
}

void updateClockDisplay() {
  struct tm timeinfo;
  if (!getLocalTime(&timeinfo)) {
    Serial.println("ERROR: Failed to get local time!");
    return;
  }

  // Display digital time
  displayDigitalTime(timeinfo);
}

void displayDigitalTime(struct tm &timeinfo) {
  // Format time string
  char timeStr[16];
  sprintf(timeStr, "%02d:%02d:%02d", timeinfo.tm_hour, timeinfo.tm_min, timeinfo.tm_sec);
  writeText(timeStr, true);
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
    TFT_display.drawRGBBitmap(x, ICON_Y, icon, ICON_WIDTH, ICON_HEIGHT);
  }
  else {
    TFT_display.fillRect(x, ICON_Y, ICON_WIDTH, ICON_HEIGHT, COLOR_BACKGROUND);
  }
}

static void updateWifiIcon(AppState state) {
  bool visible = false;
  bool okIcon  = true;

  switch (state) {
    case CONNECTED_NOT_SYNCED:
    case CONNECTED_SYNCING:
    case CONNECTED_SYNCED:
    case RESET_PENDING:
      visible = true;
      okIcon = true;
      break;
    case CONNECTING:
      visible = blinkState;
      okIcon  = true;
      break;
    case NOT_CONFIGURED:
    case DISCONNECTED:
      visible = true;
      okIcon = false;
      break;
  }

  drawIcon(visible, CENTER_X - 2 - ICON_WIDTH, okIcon ? IconWifiBitmap : IconWifiOffBitmap);
}

static void updateSyncIcon(AppState state) {
  bool visible = (state == CONNECTED_SYNCING) ? blinkState : false;
  drawIcon(visible, CENTER_X + 2, IconSyncBitmap);
}

void updateIcons() {
  unsigned long now = millis();
  if (now - lastBlink >= BLINK_INTERVAL_MS) {
    blinkState = !blinkState;
    lastBlink  = now;
  }

  AppState state = getAppState();
  updateWifiIcon(state);
  updateSyncIcon(state);
}

