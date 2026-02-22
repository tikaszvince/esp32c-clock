#include <WiFi.h>
#include <OneButton.h>

#include "icons.h"
#include "config.h"
#include "button.h"
#include "pins.h"
#include "display_constants.h"
#include "display.h"
#include "ntp.h"

void setup() {
  // Initialize serial communication
  Serial.begin(115200);
  delay(1500);

  buttonSetup();

  Serial.println("\n\nESP32 WiFi Clock");
  Serial.println("=================");

  // Initialize TFT display
  TFT_display.begin();
  TFT_display.setRotation(0);
  TFT_display.fillScreen(COLOR_BACKGROUND);

  // Show WiFi setup instructions
  if (!loadConfig()) {
    displayWifiSetupInstructions();
  }
  else {
    // Draw initial clock face
    drawClockFace();
    updateIcons();

    // Show WiFi setup message
    redrawTextBox("WiFi Setup");

    iconStatusWifi = IconStatus::flash;
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
    iconStatusWifi = IconStatus::show;
    updateIcons();
    delay(30000UL);
    ESP.restart();
  }

  // Draw initial clock face
  drawClockFace();
  iconStatusWifi = IconStatus::show;
  updateIcons();

  // Sync time with NTP server
  redrawTextBox("NTP Sync...");
  syncTimeWithNTP([](const char* msg){ writeText(msg, true); });

  // Draw clock display
  drawTextBox();
  updateClockDisplay();

  Serial.println("Setup complete!");
}

void loop() {
  // Update clock display every second
  static unsigned long lastUpdate = 0;
  static ButtonMode lastMode = NORMAL;
  unsigned long currentMillis = millis();

  ButtonMode mode = getButtonMode();

  // React to mode changes
  if (mode != lastMode) {
    if (mode == RESET_PENDING) {
      TFT_display.fillScreen(COLOR_BACKGROUND);
      displayResetQuestion();
    }
    else if (mode == NORMAL) {
      drawClockFace();
      iconStatusSync = (getNtpStatus() == NTP_SYNCING) ? IconStatus::flash : IconStatus::hide;
      updateIcons();
    }
    lastMode = mode;
  }

  if (mode == NORMAL) {
    if (currentMillis - lastUpdate >= 1000) {
      lastUpdate = currentMillis;
      // Serial.println("Updating display...");
      updateClockDisplay();
      iconStatusSync = (getNtpStatus() == NTP_SYNCING) ? IconStatus::flash : IconStatus::hide;
      updateIcons();
    }

    if (currentMillis - lastNtp >= NTP_SYNC_INTERVAL) {
      syncTimeWithNTP([](const char* msg){ writeText(msg, true); });
    }
  }

  buttonLoop();
}
