#include <WiFi.h>
#include "Arduino.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"

#include "wifi_monitor.h"
#include "config.h"
#include "app_state.h"
#include "timing_constants.h"

static TaskHandle_t wifiMonitorTaskHandle = NULL;

static void wifiMonitorTask(void* parameter) {
  for (;;) {
    vTaskDelay(pdMS_TO_TICKS(WIFI_MONITOR_CHECK_INTERVAL_MS));

    AppState state = getAppState();

    if (state == RESET_PENDING || state == CONNECTING || state == NOT_CONFIGURED) {
      continue;
    }

    if (WiFi.status() != WL_CONNECTED) {
      Serial.println("WiFi connection lost.");
      setAppState(DISCONNECTED);

      if (isReconnectDue()) {
        Serial.println("Attempting to reconnect...");
        updateLastReconnectAttempt();
        connectWifi();
      }
    }
    else if (state == DISCONNECTED) {
      Serial.println("WiFi reconnected.");
      setAppState(CONNECTED_NOT_SYNCED);
    }
  }
}

void wifiMonitorTaskStart() {
  xTaskCreatePinnedToCore(
    wifiMonitorTask,
    "WifiMonitor",
    4096,
    NULL,
    1,
    &wifiMonitorTaskHandle,
    0  // core 0
  );
  Serial.println("WiFi monitor task started on core 0.");
}
