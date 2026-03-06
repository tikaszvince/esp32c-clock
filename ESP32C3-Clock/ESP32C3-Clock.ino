#include <time.h>
#include <WiFi.h>
#include <OneButton.h>
#include "app_state.h"
#include "config.h"
#include "button.h"
#include "pins.h"
#include "display.h"
#include "clock_face_factory.h"
#include "display_task.h"
#include "timing_constants.h"

#if SCREENSHOT_MODE
  #include "screenshot_server.h"
#else
  #include "startup_screen.h"
  #include "ntp.h"
  #include "wifi_monitor.h"
#endif

void setup() {
  // Initialize serial communication
  Serial.begin(115200);
  delay(1500);

  Serial.println("\n\nESP32 WiFi Clock");
  Serial.println("=================");
  Serial.print("Chip type:       "); Serial.println(ESP.getChipModel());
  Serial.print("Flash chip size: "); Serial.println(ESP.getFlashChipSize());
  Serial.print("Free heap:       "); Serial.println(ESP.getFreeHeap());
  Serial.println("=================");

  // Initialize TFT display.
  displaySetup();
  #if !SCREENSHOT_MODE
    startupScreenTaskStart();

    // Register interactions.
    buttonSetup(
      []() { resetConfig(); },
      []() {
        AppState state = getAppState();
        if (state == CONNECTED_SYNCED || state == CONNECTED_NOT_SYNCED) {
          requestNtpSync();
        }
      }
    );
  #endif

  takeDisplayMutex();
  TFT_display.begin();
  TFT_display.setRotation(0);
  TFT_display.fillScreen(COLOR_BACKGROUND);
  giveDisplayMutex();

  #if SCREENSHOT_MODE
    Serial.println("=================");
    Serial.print("SCREENSHOT MODE ACTIVE");
    Serial.print("=================");
    loadConfig();
    if (!connectWifi()) {
      Serial.println("WiFi connection failed!");
    }

    setAppState(CONNECTED_SYNCED);
    Serial.print("Largest free contiguous block: ");
    Serial.println(heap_caps_get_largest_free_block(MALLOC_CAP_8BIT));
    screenshotServerSetup();
  #else
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

    // Initialize NTP sync.
    syncTimeWithNTP([](const char* msg) {
      setStatusText(msg, 3000);
      Serial.print("NTP status: ");
      Serial.println(msg);
    });

    // Start NTP Sync  task.
    wifiMonitorTaskStart();
    ntpTaskStart();
  #endif

  #if SCREENSHOT_MODE
    setClockFace(getInstance(SCREENSHOT_FACE));
  #else
    // TODO: load last used clockface.
    setClockFace(getInstance(CLOCK_FACE_ORBIT));
  #endif

  setInited();
  Serial.println("Setup complete!");
}

void loop() {
  #if SCREENSHOT_MODE
    redrawDisplay();
    screenshotServerLoop();
  #else
    static unsigned long lastRedraw = 0;
    unsigned long currentMillis = millis();

    if (currentMillis - lastRedraw >= BLINK_INTERVAL_MS) {
      lastRedraw = currentMillis;
      takeDisplayMutex();
      redrawDisplay();
      giveDisplayMutex();
    }

    buttonLoop();
  #endif
}
