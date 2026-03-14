#include "Arduino.h"
#include "config.h"
#include "app_state.h"
#include "timezones.h"
#if !DISABLE_ENCODER
  #include "clock_face_factory.h"
#endif

static String ntp_server = "pool.ntp.org";
static String timezone = "Europe/Budapest";

static char timezoneSelectBuf[TIMEZONE_SELECT_BUFFER_SIZE];
static char powersafeSelectBuf[256];
#if !DISABLE_ENCODER
  static char faceSelectBuf[512];
#endif

const char* WIFI_HOTSPOT_SSID = "ESP32-Clock";
const char* WIFI_HOTSPOT_PASSWORD = "clocksetup";

static Preferences preferences;
static char timezone_buffer[100];
static char ntp_server_buffer[50];
static bool shouldSaveConfig = false;
static bool powersafe_mode = true;
static String default_face_id = "orbit";

static void saveConfigCallback() {
  Serial.println("Should save config");
  shouldSaveConfig = true;
}

bool isIanaFormat(const char* tz) {
  if (tz == nullptr || strlen(tz) == 0) {
    return false;
  }

  // Rule out POSIX formats by checking what they contain:

  // 1. POSIX DST rules use commas (e.g., "CET-1CEST,M3.5.0,M10.5.0/3")
  //    IANA timezones never use commas
  if (strchr(tz, ',') != nullptr) {
    return false;  // Has comma → POSIX
  }

  // 2. POSIX uses offset notation: + or - followed by a digit
  //    (e.g., "CET-1CEST", "UTC+5", "GMT-3")
  //    IANA never has this pattern
  for (int i = 0; tz[i] != '\0'; i++) {
    if ((tz[i] == '+' || tz[i] == '-') && tz[i + 1] != '\0' && isdigit(tz[i + 1])) {
      return false;  // Has offset → POSIX
    }
  }

  // 3. IANA must contain at least one '/' (e.g., "Europe/Budapest")
  //    Simple POSIX like "UTC0", "GMT0", "PST8PDT" don't have slashes
  if (strchr(tz, '/') == nullptr) {
    return false;  // No slash → POSIX
  }

  // Passed all POSIX exclusion checks → treat as IANA
  return true;
}

bool loadConfig() {
  preferences.begin("clock-config", true);

  // Changed: use "timezone_iana" key for new format
  // Keep "timezone" key for backward compatibility
  String saved_tz_iana = preferences.getString("timezone_iana", "");
  String saved_tz_legacy = preferences.getString("timezone", "");
  String saved_ntp = preferences.getString("ntp_server", "pool.ntp.org");
  bool wifiConfigured = preferences.getBool("wifi_configured", false);
  bool saved_powersafe = preferences.getBool("powersafe", true);
  powersafe_mode = saved_powersafe;
  String saved_face_id = preferences.getString("default_face", "orbit");
  default_face_id = saved_face_id;
  Serial.print("Loaded default face: ");
  Serial.println(default_face_id);

  preferences.end();

  // Migration logic: prefer IANA, fall back to legacy POSIX
  if (saved_tz_iana.length() > 0) {
    // Modern IANA format
    strcpy(timezone_buffer, saved_tz_iana.c_str());
    timezone = saved_tz_iana;
    Serial.println("Loaded IANA timezone: " + timezone);
  }
  else if (saved_tz_legacy.length() > 0) {
    // Legacy POSIX format - keep it but log migration needed
    strcpy(timezone_buffer, saved_tz_legacy.c_str());
    timezone = saved_tz_legacy;
    Serial.println("Loaded legacy POSIX timezone: " + timezone);
    Serial.println("Will migrate to IANA on next config save");
  }
  else {
    // No stored timezone, use default
    strcpy(timezone_buffer, "Europe/Budapest");
    timezone = "Europe/Budapest";
    Serial.println("Using default IANA timezone: " + timezone);
  }

  strcpy(ntp_server_buffer, saved_ntp.c_str());
  ntp_server = saved_ntp;

  Serial.print("Loaded NTP server: ");
  Serial.println(ntp_server_buffer);
  Serial.print("WiFi previously configured: ");
  Serial.println(wifiConfigured);

  return wifiConfigured;
}

static void buildPowersafeSelect(bool currentValue, char* buf, int bufSize) {
  snprintf(
    buf,
    bufSize,
    "<label for='powersafe'>Power safe mode. Disable networking when not needed</label>"
    "<select name='powersafe'>"
    "<option value='1'%s>Enabled</option>"
    "<option value='0'%s>Disabled</option>"
    "</select>",
    currentValue ? " selected" : "",
    currentValue ? "" : " selected"
  );
}

#if !DISABLE_ENCODER
  static void buildFaceSelect(const char* currentId, char* buf, int bufSize) {
    int pos = 0;
    pos += snprintf(buf + pos, bufSize - pos,
      "<label for='default_face'>Default clock face</label>"
      "<select name='default_face'>"
    );
    for (int i = 0; i < getFaceCount(); i++) {
      ClockFace* face = getFaceAt(i);
      bool selected = strcmp(face->getId(), currentId) == 0;
      pos += snprintf(buf + pos, bufSize - pos,
        "<option value='%s'%s>%s</option>",
        face->getId(),
        selected ? " selected" : "",
        face->getName()
      );
    }
    snprintf(buf + pos, bufSize - pos, "</select>");
  }
#endif

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

  buildPowersafeSelect(powersafe_mode, powersafeSelectBuf, sizeof(powersafeSelectBuf));
  WiFiManagerParameter custom_powersafe(powersafeSelectBuf);
  wm.addParameter(&custom_powersafe);

  #if !DISABLE_ENCODER
    buildFaceSelect(default_face_id.c_str(), faceSelectBuf, sizeof(faceSelectBuf));
    WiFiManagerParameter custom_face_select(faceSelectBuf);
    wm.addParameter(&custom_face_select);
  #endif

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

    #if !DISABLE_ENCODER
      const char* newFaceId = custom_face_select.getValue();
      if (newFaceId != nullptr && strlen(newFaceId) > 0) {
        default_face_id = String(newFaceId);
      }
    #endif

    timezone = String(timezone_buffer);
    ntp_server = String(ntp_server_buffer);

    // Validate the IANA timezone
    if (!isIanaFormat(timezone_buffer)) {
      Serial.println("Warning: Received non-IANA timezone format, using default");
      strcpy(timezone_buffer, "Europe/Budapest");
      timezone = "Europe/Budapest";
    }
  }

  // Save everything including the wifi_configured flag
  preferences.begin("clock-config", false);
  preferences.putString("timezone_iana", timezone_buffer);
  preferences.putString("ntp_server", ntp_server_buffer);
  preferences.putBool("wifi_configured", true);
  preferences.putBool("powersafe", powersafe_mode);
  preferences.putString("default_face", default_face_id.c_str());

  // Clean up legacy key on save (migration)
  preferences.remove("timezone");

  preferences.end();

  Serial.println("\nWiFi connected!");
  Serial.print("IP Address: ");
  Serial.println(WiFi.localIP());
  Serial.print("Signal Strength: ");
  Serial.print(WiFi.RSSI());
  Serial.println(" dBm");
  Serial.print("Saved IANA timezone: ");
  Serial.println(timezone_buffer);

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

String getTimezone() {
  // If stored value is IANA format, convert to POSIX
  if (isIanaFormat(timezone.c_str())) {
    const char* posix = ianaToPosix(timezone.c_str());
    return String(posix);
  }

  // Legacy POSIX format - return as-is (migration fallback)
  Serial.println("Warning: Using legacy POSIX timezone string");
  return timezone;
}

String getTimezoneIana() {
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

String getDefaultFaceId() {
  return default_face_id;
}

void saveDefaultFaceId(const char* id) {
  if (id == nullptr || strlen(id) == 0) {
    return;
  }
  default_face_id = String(id);
  preferences.begin("clock-config", false);
  preferences.putString("default_face", id);
  preferences.end();
  Serial.print("Default face saved: ");
  Serial.println(id);
}
