#include <WiFi.h>
#include <OneButton.h>

#include "icons.h"
#include "app_state.h"
#include "config.h"
#include "button.h"
#include "pins.h"
#include "display_constants.h"
#include "display.h"
#include "ntp.h"
#include "display_task.h"
#include "wifi_monitor.h"

void setup() {
  // Initialize serial communication
  Serial.begin(115200);
  delay(1500);

  buttonSetup(
    []() { resetConfig(); },
    []() {
      AppState state = getAppState();
      if (state == CONNECTED_SYNCED || state == CONNECTED_NOT_SYNCED) {
        requestNtpSync();
      }
    }
  );

  Serial.println("\n\nESP32 WiFi Clock");
  Serial.println("=================");

  // Initialize TFT display.
  displaySetup();
  takeDisplayMutex();
  TFT_display.begin();
  TFT_display.setRotation(0);
  TFT_display.fillScreen(COLOR_BACKGROUND);
  giveDisplayMutex();

  displayTaskStart();

  // Show WiFi setup instructions.
  if (!loadConfig()) {
    setAppState(NOT_CONFIGURED);
    takeDisplayMutex();
    displayWifiSetupInstructions();
    giveDisplayMutex();
  }
  else {
    takeDisplayMutex();
    // Draw initial clock face
    drawClockFace();
    // Show WiFi setup message
    redrawTextBox("WiFi Setup");
    updateIcons();
    giveDisplayMutex();
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
    updateIcons();
    delay(30000UL);
    ESP.restart();
  }

  // Draw initial clock face
  takeDisplayMutex();
  drawClockFace();
  updateIcons();
  giveDisplayMutex();

  // Start NTP Sync  task.
  wifiMonitorTaskStart();
  ntpTaskStart();

  // Draw clock display
  takeDisplayMutex();
  drawTextBox();
  updateClockDisplay();
  giveDisplayMutex();

  displayTaskStop();
  setInited();
  Serial.println("Setup complete!");
}

void loop() {
  // Update clock display every second.
  static unsigned long lastUpdate = 0;
  static unsigned long lastIconUpdate = 0;
  static AppState lastState = NOT_CONFIGURED;
  unsigned long currentMillis = millis();

  AppState state = getAppState();

  // React to mode changes
  if (state != lastState) {
    if (state == RESET_PENDING) {
      takeDisplayMutex();
      TFT_display.fillScreen(COLOR_BACKGROUND);
      displayResetQuestion();
      giveDisplayMutex();
    }
    else if (lastState == RESET_PENDING) {
      takeDisplayMutex();
      drawClockFace();
      giveDisplayMutex();
    }
    lastState = state;
  }

  if (state != RESET_PENDING) {
    if (currentMillis - lastUpdate >= 1000) {
      lastUpdate = currentMillis;
      takeDisplayMutex();
      updateClockDisplay();
      giveDisplayMutex();
    }

    if (currentMillis - lastIconUpdate >= BLINK_INTERVAL_MS) {
      lastIconUpdate = currentMillis;
      takeDisplayMutex();
      updateIcons();
      giveDisplayMutex();
    }
  }

  buttonLoop();
}
