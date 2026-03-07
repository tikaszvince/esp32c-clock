#include "Arduino.h"
#include "config.h"
#include "app_state.h"
#include "timezones.h"

static String ntp_server = "pool.ntp.org";
static String timezone = "CET-1CEST,M3.5.0,M10.5.0/3";

static char timezoneSelectBuf[TIMEZONE_SELECT_BUFFER_SIZE];

const char* WIFI_HOTSPOT_SSID = "ESP32-Clock";
const char* WIFI_HOTSPOT_PASSWORD = "clocksetup";

static Preferences preferences;
static char timezone_buffer[100];
static char ntp_server_buffer[50];
static bool shouldSaveConfig = false;
static bool powersafe_mode = true;

static void saveConfigCallback() {
  Serial.println("Should save config");
  shouldSaveConfig = true;
}

bool loadConfig() {
  preferences.begin("clock-config", true);
  String saved_tz = preferences.getString("timezone", "CET-1CEST,M3.5.0,M10.5.0/3");
  String saved_ntp = preferences.getString("ntp_server", "pool.ntp.org");
  bool wifiConfigured = preferences.getBool("wifi_configured", false);
  bool saved_powersafe = preferences.getBool("powersafe", true);
  powersafe_mode = saved_powersafe;

  preferences.end();

  strcpy(timezone_buffer, saved_tz.c_str());
  strcpy(ntp_server_buffer, saved_ntp.c_str());

  timezone = saved_tz;
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
  setAppState(CONNECTING);

  WiFiManager wm;
  shouldSaveConfig = false;

  buildTimezoneSelect("timezone", timezone_buffer, timezoneSelectBuf, TIMEZONE_SELECT_BUFFER_SIZE);
  WiFiManagerParameter custom_timezone_select(timezoneSelectBuf);
  WiFiManagerParameter custom_ntp_server("ntp_server", "NTP Server", ntp_server_buffer, 50);

  wm.addParameter(&custom_timezone_select);
  wm.addParameter(&custom_ntp_server);
  wm.setSaveParamsCallback(saveConfigCallback);
  wm.setConfigPortalTimeout(180);

  char powersafeHtml[256];
  snprintf(
    powersafeHtml,
    sizeof(powersafeHtml),
    "<label for='powersafe'>Power safe mode. Disable networking when not needed</label>"
    "<select name='powersafe'>"
    "<option value='1'%s>Enabled</option>"
    "<option value='0'%s>Disabled</option>"
    "</select>",
    powersafe_mode ? " selected" : "",
    powersafe_mode ? "" : " selected"
  );
  WiFiManagerParameter custom_powersafe(powersafeHtml);
  wm.addParameter(&custom_powersafe);

  Serial.println("Calling autoConnect...");
  bool connected = wm.autoConnect(WIFI_HOTSPOT_SSID, WIFI_HOTSPOT_PASSWORD);

  if (!connected) {
    Serial.println("Failed to connect to WiFi");
    setAppState(DISCONNECTED);
    return false;
  }

  if (shouldSaveConfig) {
    Serial.println("Saving new configuration...");
    strcpy(timezone_buffer, custom_timezone_select.getValue());
    strcpy(ntp_server_buffer, custom_ntp_server.getValue());
    powersafe_mode = strcmp(custom_powersafe.getValue(), "1") == 0;

    timezone = String(timezone_buffer);
    ntp_server = String(ntp_server_buffer);
  }

  // Save everything including the wifi_configured flag
  preferences.begin("clock-config", false);
  preferences.putString("timezone", timezone_buffer);
  preferences.putString("ntp_server", ntp_server_buffer);
  preferences.putBool("wifi_configured", true);
  preferences.putBool("powersafe", powersafe_mode);
  preferences.end();

  Serial.println("\nWiFi connected!");
  Serial.print("IP Address: ");
  Serial.println(WiFi.localIP());
  Serial.print("Signal Strength: ");
  Serial.print(WiFi.RSSI());
  Serial.println(" dBm");

  setAppState(CONNECTED_NOT_SYNCED);
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

String getTimezone()  {
  return timezone;
}
String getNTPServer() {
  return ntp_server;
}

bool getPowersafeMode() {
  return powersafe_mode;
}

bool reconnectWifi() {
  Serial.println("Reconnecting to WiFi...");
  setAppState(CONNECTING);

  WiFi.mode(WIFI_STA);
  WiFi.begin();

  int attempts = 0;
  while (WiFi.status() != WL_CONNECTED && attempts < 20) {
    delay(500);
    Serial.print(".");
    attempts++;
  }

  if (WiFi.status() == WL_CONNECTED) {
    Serial.println("\nWiFi reconnected!");
    Serial.print("IP Address: ");
    Serial.println(WiFi.localIP());
    setAppState(CONNECTED_NOT_SYNCED);
    return true;
  }

  Serial.println("\nReconnect failed, falling back to full connect...");
  return connectWifi();
}
