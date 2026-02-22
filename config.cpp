#include "Arduino.h"
#include "config.h"
#include "states.h"

static String ntp_server = "pool.ntp.org";
static String timezone   = "CET-1CEST,M3.5.0,M10.5.0/3";

const char* WIFI_HOTSPOT_SSID     = "ESP32-Clock";
const char* WIFI_HOTSPOT_PASSWORD = "clocksetup";

static Preferences preferences;
static char timezone_buffer[100];
static char ntp_server_buffer[50];
static bool shouldSaveConfig = false;
static WifiState currentWifiState = WIFI_DISCONNECTED;

static void saveConfigCallback() {
  Serial.println("Should save config");
  shouldSaveConfig = true;
}

WifiState getWifiState() { return currentWifiState; }

bool loadConfig() {
  preferences.begin("clock-config", true);
  String saved_tz  = preferences.getString("timezone",   "CET-1CEST,M3.5.0,M10.5.0/3");
  String saved_ntp = preferences.getString("ntp_server", "pool.ntp.org");
  bool wifiConfigured = preferences.getBool("wifi_configured", false);
  preferences.end();

  strcpy(timezone_buffer,   saved_tz.c_str());
  strcpy(ntp_server_buffer, saved_ntp.c_str());

  timezone   = saved_tz;
  ntp_server = saved_ntp;

  Serial.print("Loaded timezone: ");
  Serial.println(timezone_buffer);
  Serial.print("Loaded NTP server: ");
  Serial.println(ntp_server_buffer);
  Serial.print("WiFi previously configured: ");
  Serial.println(wifiConfigured);

  return wifiConfigured;
}

bool connectWifi() {
  currentWifiState = WIFI_CONNECTING;
  WiFiManager wm;
  shouldSaveConfig = false;

  WiFiManagerParameter custom_timezone(  "timezone",   "Timezone (POSIX format)", timezone_buffer,   100);
  WiFiManagerParameter custom_ntp_server("ntp_server", "NTP Server",              ntp_server_buffer, 50);

  wm.addParameter(&custom_timezone);
  wm.addParameter(&custom_ntp_server);
  wm.setSaveParamsCallback(saveConfigCallback);
  wm.setConfigPortalTimeout(180);

  Serial.println("Calling autoConnect...");
  bool connected = wm.autoConnect(WIFI_HOTSPOT_SSID, WIFI_HOTSPOT_PASSWORD);

  if (!connected) {
    Serial.println("Failed to connect to WiFi");
    currentWifiState = WIFI_DISCONNECTED;
    return false;
  }

  if (shouldSaveConfig) {
    Serial.println("Saving new configuration...");
    strcpy(timezone_buffer,   custom_timezone.getValue());
    strcpy(ntp_server_buffer, custom_ntp_server.getValue());

    timezone   = String(timezone_buffer);
    ntp_server = String(ntp_server_buffer);
  }

  // Save everything including the wifi_configured flag
  preferences.begin("clock-config", false);
  preferences.putString("timezone",       timezone_buffer);
  preferences.putString("ntp_server",     ntp_server_buffer);
  preferences.putBool("wifi_configured",  true);
  preferences.end();

  Serial.println("\nWiFi connected!");
  Serial.print("IP Address: "); Serial.println(WiFi.localIP());
  Serial.print("Signal Strength: "); Serial.print(WiFi.RSSI()); Serial.println(" dBm");

  currentWifiState = WIFI_CONNECTED;
  return true;
}

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

String getTimezone()  { return timezone;   }
String getNTPServer() { return ntp_server; }
