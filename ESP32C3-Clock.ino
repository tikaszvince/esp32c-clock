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

static void ntpStatusCallback(const char* msg) {
  writeText(msg, true);
}

void setup() {
  // Initialize serial communication
  Serial.begin(115200);
  delay(1500);

  buttonSetup(
    []() { resetConfig(); }
  );

  Serial.println("\n\nESP32 WiFi Clock");
  Serial.println("=================");

  // Initialize TFT display.
  TFT_display.begin();
  TFT_display.setRotation(0);
  TFT_display.fillScreen(COLOR_BACKGROUND);

  // Show WiFi setup instructions.
  if (!loadConfig()) {
    setAppState(NOT_CONFIGURED);
    displayWifiSetupInstructions();
  }
  else {
    // Draw initial clock face
    drawClockFace();
    // Show WiFi setup message
    redrawTextBox("WiFi Setup");
    updateIcons();
  }

  // Initialize WiFi configuration (web portal)
  if (!connectWifi()) {
    TFT_display.fillScreen(COLOR_BACKGROUND);
    TFT_display.setTextColor(COLOR_RED, COLOR_BACKGROUND);
    TFT_display.setTextSize(2);
    TFT_display.setCursor((SCREEN_WIDTH - (strlen("WiFi Failed") * 12)) / 2, CENTER_Y - 20);
    TFT_display.print("WiFi Failed");
    TFT_display.setTextSize(1);
    TFT_display.setCursor((SCREEN_WIDTH - (strlen("Restarting...") * 6)) / 2, CENTER_Y + 15);
    TFT_display.print("Restarting...");
    updateIcons();
    delay(30000UL);
    ESP.restart();
  }

  // Draw initial clock face
  drawClockFace();
  updateIcons();

  // Sync time with NTP server
  redrawTextBox("NTP Sync...");
  syncTimeWithNTP(ntpStatusCallback);

  // Draw clock display
  drawTextBox();
  updateClockDisplay();

  setInited();
  Serial.println("Setup complete!");
}

void loop() {
  // Update clock display every second.
  static unsigned long lastUpdate = 0;
  static AppState lastState = NOT_CONFIGURED;
  unsigned long currentMillis = millis();

  AppState state = getAppState();

  // React to mode changes
  if (state != lastState) {
    if (state == RESET_PENDING) {
      TFT_display.fillScreen(COLOR_BACKGROUND);
      displayResetQuestion();
    }
     else if (lastState == RESET_PENDING) {
      drawClockFace();
    }
    lastState = state;
  }

  if (state != RESET_PENDING) {
    if (currentMillis - lastUpdate >= 1000) {
      lastUpdate = currentMillis;
      updateClockDisplay();
    }

    if (isNtpSyncDue()) {
      syncTimeWithNTP(ntpStatusCallback);
    }
    updateIcons();
  }
  buttonLoop();
}
