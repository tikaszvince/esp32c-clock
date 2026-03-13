#ifndef CONFIG_H
#define CONFIG_H

#include <WiFiManager.h>
#include <Preferences.h>

extern const char* WIFI_HOTSPOT_SSID;
extern const char* WIFI_HOTSPOT_PASSWORD;

// Exposed config getters
String getTimezone();
String getTimezoneIana();
bool isIanaFormat(const char* tz);
String getNTPServer();
String getDefaultFaceId();

// Lifecycle
// Returns true if WiFi was previously configured.
bool loadConfig();

// Returns true if WiFi connected successfully.
bool connectWifi();
void resetConfig();

// Returns true if WiFi reconnected successfully.
bool reconnectWifi();
bool getPowersafeMode();
void saveDefaultFaceId(const char* id);

#endif
