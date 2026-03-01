#include <math.h>
#include "Arduino.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"

#include "startup_screen.h"
#include "display.h"
#include "app_state.h"
#include "timing_constants.h"
#include "icons.h"

static TaskHandle_t startupScreenTaskHandle = NULL;

static const int SPINNER_X = 120;
static const int SPINNER_Y = 80;
static const char SPINNER_CHARS[] = {'|', '/', '-', '\\'};
//static const char SPINNER_CHARS[] = {'▙', '▛', '▜', '▟'};
static const int SPINNER_STEPS = 4;

static const int ICON_SIZE = 24;
static const int STARTUP_ICON_WIFI_X = 88;
static const int STARTUP_ICON_WIFI_Y = 130;
static const int STARTUP_ICON_NTP_X = 128;
static const int STARTUP_ICON_NTP_Y = 130;

static void drawSpinner(int step) {
  char buf[2] = {SPINNER_CHARS[step], '\0'};
  TFT_display.setTextSize(3);
  TFT_display.setTextColor(COLOR_YELLOW, COLOR_BACKGROUND);
  TFT_display.setCursor(SPINNER_X - 9, SPINNER_Y - 12);
  TFT_display.print(buf);
}

static void drawIcon(bool visible, int x, int y, const uint16_t* bitmap) {
  if (visible) {
    TFT_display.drawRGBBitmap(x, y, bitmap, ICON_SIZE, ICON_SIZE);
  }
  else {
    TFT_display.fillRect(x, y, ICON_SIZE, ICON_SIZE, COLOR_BACKGROUND);
  }
}

static void startupScreenTask(void* parameter) {
  int spinnerStep = 0;
  bool blinkState = false;

  takeDisplayMutex();
  TFT_display.fillScreen(COLOR_BACKGROUND);
  giveDisplayMutex();

  for (;;) {
    AppState state = getAppState();

    if (
      isInited()
      || state == DISCONNECTED
      || state == CONNECTED_NOT_SYNCED
      || state == CONNECTED_SYNCED
    ) {
      Serial.print("Startup screen should terminate");
      break;
    }

    bool showWifi = false;
    bool blinkWifi = false;
    bool showNtp = false;
    bool blinkNtp = false;

    switch (state) {
      case CONNECTING:
        showWifi = true;
        blinkWifi = true;
        break;
      case CONNECTED_SYNCING:
        showWifi = true;
        blinkWifi = false;
        showNtp = true;
        blinkNtp = true;
        break;
      default:
        break;
    }

    takeDisplayMutex();
    drawSpinner(spinnerStep);
    drawIcon(showWifi && (!blinkWifi || blinkState), STARTUP_ICON_WIFI_X, STARTUP_ICON_WIFI_Y, IconWifiBitmap);
    drawIcon(showNtp && (!blinkNtp || blinkState), STARTUP_ICON_NTP_X, STARTUP_ICON_NTP_Y, IconSyncBitmap);
    giveDisplayMutex();

    spinnerStep = (spinnerStep + 1) % SPINNER_STEPS;
    blinkState = !blinkState;

    vTaskDelay(pdMS_TO_TICKS(BLINK_INTERVAL_MS));
  }

  Serial.print("Startup screen ends: ");
  if (isInited()) {
    Serial.println("app inited");
  }
  else {
    Serial.println("app state changed to: ");
    Serial.println(getAppState());
  }
  vTaskDelete(NULL);
}

void startupScreenTaskStart() {
  xTaskCreatePinnedToCore(
    startupScreenTask,
    "StartupScreen",
    4096,
    NULL,
    1,
    &startupScreenTaskHandle,
    1  // core 1
  );
  Serial.println("Startup screen task started on core 1.");
}
