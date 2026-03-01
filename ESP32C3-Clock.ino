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
#include "timing_constants.h"

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
    takeDisplayMutex();
    displayWifiSetupInstructions();
    giveDisplayMutex();
  }

  // Initialize WiFi configuration (web portal)
  if (!connectWifi()) {
    takeDisplayMutex();
    displayWifiError();
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
