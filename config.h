#ifndef CONFIG_H
#define CONFIG_H

#include <WiFiManager.h>
#include <Preferences.h>

extern const char* WIFI_HOTSPOT_SSID;
extern const char* WIFI_HOTSPOT_PASSWORD;

// Exposed config getters
String getTimezone();
String getNTPServer();
bool hasStoredWifiCredentials();

// Lifecycle
bool loadConfig();   // returns true if WiFi was previously configured
bool connectWifi();  // returns true if WiFi connected successfully
void resetConfig();

#endif
