#include "Arduino.h"
#include "ntp.h"
#include "config.h"
#include <WiFi.h>
#include <time.h>

const unsigned long NTP_SYNC_INTERVAL = 3UL * 60UL * 60UL * 1000UL;
unsigned long lastNtp = 0;

static NtpStatus currentNtpStatus = NTP_IDLE;

NtpStatus getNtpStatus() {
  return currentNtpStatus;
}

void syncTimeWithNTP(void (*onStatus)(const char*)) {
  currentNtpStatus = NTP_SYNCING;

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
        lastNtp = millis();
        onStatus("Time synced");
        break;
      }
      else {
        Serial.println(" Failed!");
      }
    }

    if (!timeSet) {
      onStatus("Sync failed");
      Serial.println("Failed with all NTP servers!");
    }
  }
  else {
    onStatus("No WiFi");
    Serial.println("Cannot sync - WiFi not connected!");
  }

  currentNtpStatus = NTP_IDLE;
}
