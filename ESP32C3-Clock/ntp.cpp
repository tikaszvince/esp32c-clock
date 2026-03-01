#include <WiFi.h>
#include <time.h>
#include "Arduino.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"

#include "ntp.h"
#include "config.h"
#include "app_state.h"
#include "timing_constants.h"

static TaskHandle_t ntpTaskHandle = NULL;

static void ntpStatusCallback(const char* msg) {
  setStatusText(msg, 3000);
  Serial.print("NTP status: ");
  Serial.println(msg);
}

static void ntpTask(void* parameter) {
  for (;;) {
    AppState state = getAppState();

    if (state == CONNECTED_SYNCED || state == CONNECTED_NOT_SYNCED) {
      if (isNtpSyncRequested() || isNtpSyncDue()) {
        clearNtpSyncRequest();
        syncTimeWithNTP(ntpStatusCallback);
      }
    }

    vTaskDelay(pdMS_TO_TICKS(NTP_TASK_CHECK_INTERVAL_MS));
  }
}

void ntpTaskStart() {
  xTaskCreatePinnedToCore(
    ntpTask,
    "NtpTask",
    4096,
    NULL,
    1,
    &ntpTaskHandle,
    0  // core 0
  );
  Serial.println("NTP task started on core 0.");
}

void syncTimeWithNTP(void (*onStatus)(const char*)) {
  setAppState(CONNECTED_SYNCING);

  if (WiFi.status() == WL_CONNECTED) {
    Serial.println("\nSynchronizing time with NTP server...");

    const char* ntpServerList[] = {
      getNTPServer().c_str(),
      "pool.ntp.org",
      "time.google.com",
      "time.cloudflare.com",
      "time.windows.com",
      "hu.pool.ntp.org"
    };

    bool timeSet = false;

    for (int serverIdx = 0; serverIdx < 6 && !timeSet; serverIdx++) {
      const char* server = ntpServerList[serverIdx];

      if (server == NULL || strlen(server) == 0) {
        Serial.println("Skipping empty NTP server");
        continue;
      }

      Serial.print("Trying NTP server: ");
      Serial.println(server);

      configTzTime(getTimezone().c_str(), server);

      struct tm timeinfo;
      int attempts = 0;
      while (!getLocalTime(&timeinfo) && attempts < 10) {
        Serial.print(".");
        delay(500);
        yield();
        attempts++;
      }

      if (getLocalTime(&timeinfo)) {
        Serial.println("\nTime synchronized successfully!");
        Serial.printf(
          "Time: %02d:%02d:%02d\n",
          timeinfo.tm_hour,
          timeinfo.tm_min,
          timeinfo.tm_sec
        );
        timeSet = true;
        updateLastNtpSync();
        onStatus("Time synced");
        delay(1000);
        break;
      }
      else {
        Serial.println(" Failed!");
      }
    }

    if (!timeSet) {
      setAppState(CONNECTED_NOT_SYNCED);
      onStatus("Sync failed");
      Serial.println("Failed with all NTP servers!");
    }
    else {
      setAppState(CONNECTED_SYNCED);
    }
  }
  else {
    onStatus("No WiFi");
    Serial.println("Cannot sync - WiFi not connected!");
    setAppState(DISCONNECTED);
  }
}
