/*
 * config.h - Configuration Management
 * Handles WiFi and timezone configuration using WiFiManager web portal
 */

#ifndef CONFIG_H
#define CONFIG_H

#include <WiFiManager.h>
#include <Preferences.h>

// Configuration variables
String ntp_server = "pool.ntp.org";
String timezone = "CET-1CEST,M3.5.0,M10.5.0/3";

// Preferences object for persistent storage
Preferences preferences;

// Custom parameters for WiFiManager
char timezone_buffer[100];
char ntp_server_buffer[50];

// Flag to track if config was saved
bool shouldSaveConfig = false;

// Callback when WiFiManager saves new config
void saveConfigCallback() {
  Serial.println("Should save config");
  shouldSaveConfig = true;
}

// Initialize configuration system
// Returns true if WiFi connected, false if failed
bool initConfig() {
  Serial.println("Initializing configuration...");

  // Load saved configuration from preferences
  preferences.begin("clock-config", true); // true = read-only
  String saved_tz = preferences.getString("timezone", "CET-1CEST,M3.5.0,M10.5.0/3");
  String saved_ntp = preferences.getString("ntp_server", "pool.ntp.org");
  preferences.end();

  // Copy to buffers
  strcpy(timezone_buffer, saved_tz.c_str());
  strcpy(ntp_server_buffer, saved_ntp.c_str());

  Serial.print("Loaded timezone: ");
  Serial.println(timezone_buffer);
  Serial.print("Loaded NTP server: ");
  Serial.println(ntp_server_buffer);

  // Create WiFiManager instance
  WiFiManager wm;

  // Reset flag
  shouldSaveConfig = false;

  // Add custom parameters to config portal
  WiFiManagerParameter custom_timezone(
    "timezone",
    "Timezone (POSIX format)",
    timezone_buffer,
    100
  );

  WiFiManagerParameter custom_ntp_server(
    "ntp_server",
    "NTP Server",
    ntp_server_buffer,
    50
  );

  wm.addParameter(&custom_timezone);
  wm.addParameter(&custom_ntp_server);

  // Set callback for saving parameters
  wm.setSaveParamsCallback(saveConfigCallback);

  // Set timeout for config portal (3 minutes)
  wm.setConfigPortalTimeout(180);

  // Try to connect to saved WiFi, or start config portal
  Serial.println("Calling autoConnect...");
  bool connected = wm.autoConnect("ESP32-Clock", "clocksetup");

  if (!connected) {
    Serial.println("Failed to connect to WiFi");
    return false;
  }

  // If config was saved, update stored values
  if (shouldSaveConfig) {
    Serial.println("Saving new configuration...");

    // Get values from form
    strcpy(timezone_buffer, custom_timezone.getValue());
    strcpy(ntp_server_buffer, custom_ntp_server.getValue());

    // Save to preferences
    preferences.begin("clock-config", false);
    preferences.putString("timezone", timezone_buffer);
    preferences.putString("ntp_server", ntp_server_buffer);
    preferences.end();

    Serial.println("Configuration saved!");
  }

  // Update global variables
  timezone = String(timezone_buffer);
  ntp_server = String(ntp_server_buffer);

  Serial.println("\nWiFi connected!");
  Serial.print("IP Address: ");
  Serial.println(WiFi.localIP());
  Serial.print("Signal Strength: ");
  Serial.print(WiFi.RSSI());
  Serial.println(" dBm");

  return true;
}

// Reset WiFi settings (for testing/reconfiguration)
void resetConfig() {
  Serial.println("Resetting WiFi configuration...");
  WiFiManager wm;
  wm.resetSettings();

  preferences.begin("clock-config", false);
  preferences.clear();
  preferences.end();

  Serial.println("Configuration reset! Restarting...");
  delay(1000);
  ESP.restart();
}

// Get configuration values
String getTimezone() {
  return timezone;
}

String getNTPServer() {
  return ntp_server;
}

#endif
