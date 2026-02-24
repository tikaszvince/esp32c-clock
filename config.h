#ifndef CONFIG_H
#define CONFIG_H

#include <WiFiManager.h>
#include <Preferences.h>

extern const char* WIFI_HOTSPOT_SSID;
extern const char* WIFI_HOTSPOT_PASSWORD;

// Exposed config getters
String getTimezone();
String getNTPServer();

// Lifecycle
// Returns true if WiFi was previously configured.
bool loadConfig();

// Returns true if WiFi connected successfully.
bool connectWifi();
void resetConfig();

#endif
