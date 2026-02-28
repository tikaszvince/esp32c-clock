#include <WiFi.h>
#include <OneButton.h>

#include "app_state.h"
#include "config.h"
#include "button.h"
#include "pins.h"
#include "display.h"
#include "clock_face_factory.h"
#include "ntp.h"
#include "display_task.h"
#include "wifi_monitor.h"

void setup() {
  // Initialize serial communication
  Serial.begin(115200);
  delay(1500);

  Serial.println("\n\nESP32 WiFi Clock");
  Serial.println("=================");

  buttonSetup(
    []() { resetConfig(); },
    []() {
      AppState state = getAppState();
      if (state == CONNECTED_SYNCED || state == CONNECTED_NOT_SYNCED) {
        requestNtpSync();
      }
    }
  );

  // Initialize TFT display.
  displaySetup();
  setClockFace(getInstance(CLOCK_FACE_CLASSIC));

  takeDisplayMutex();
  TFT_display.begin();
  TFT_display.setRotation(0);
  TFT_display.fillScreen(COLOR_BACKGROUND);
  giveDisplayMutex();

  if (!loadConfig()) {
    setAppState(NOT_CONFIGURED);
  }

  // Initialize WiFi configuration (web portal)
  if (!connectWifi()) {
    takeDisplayMutex();
    TFT_display.fillScreen(COLOR_BACKGROUND);
    TFT_display.setTextColor(COLOR_RED, COLOR_BACKGROUND);
    TFT_display.setTextSize(2);
    TFT_display.setCursor((SCREEN_WIDTH - (strlen("WiFi Failed") * 12)) / 2, CENTER_Y - 20);
    TFT_display.print("WiFi Failed");
    TFT_display.setTextSize(1);
    TFT_display.setCursor((SCREEN_WIDTH - (strlen("Restarting...") * 6)) / 2, CENTER_Y + 15);
    TFT_display.print("Restarting...");
    giveDisplayMutex();
    delay(30000UL);
    ESP.restart();
  }

  // Start NTP Sync  task.
  wifiMonitorTaskStart();
  ntpTaskStart();

  setInited();
  Serial.println("Setup complete!");
}

void loop() {
  static unsigned long lastRedraw = 0;
  unsigned long currentMillis = millis();

  if (currentMillis - lastRedraw >= BLINK_INTERVAL_MS) {
    lastRedraw = currentMillis;
    takeDisplayMutex();
    redrawDisplay();
    giveDisplayMutex();
  }

  buttonLoop();
}
