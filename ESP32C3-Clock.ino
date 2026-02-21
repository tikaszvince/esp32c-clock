#include <WiFi.h>
#include <time.h>
#include <DIYables_TFT_Round.h>
#include "icons.h"
#include <math.h>
#include <LittleFS.h>
#include <ArduinoJson.h>  // Need to install this library

// TFT Display pins.
#define PIN_RST 27  // The ESP32 pin GPIO27 connected to the RST pin of the circular TFT display
#define PIN_DC  25  // The ESP32 pin GPIO25 connected to the DC pin of the circular TFT display
#define PIN_CS  26  // The ESP32 pin GPIO26 connected to the CS pin of the circular TFT display

// Colors
#define COLOR_BACKGROUND  DIYables_TFT::colorRGB(0, 0, 0)       // Blackc:\Users\Vince\Documents\Arduino\Clock\ESP32C3_Super_Mini_Bodmers_Clock\config.h
#define COLOR_CLOCKFACE   DIYables_TFT::colorRGB(255, 255, 255) // White
#define COLOR_HOUR_HAND   DIYables_TFT::colorRGB(255, 255, 255) // White
#define COLOR_MINUTE_HAND DIYables_TFT::colorRGB(80, 255, 255)  // Cyan
#define COLOR_SECOND_HAND DIYables_TFT::colorRGB(255, 0, 0)     // Red
#define COLOR_CENTER_DOT  DIYables_TFT::colorRGB(255, 0, 0)     // Red
#define COLOR_YELLOW      DIYables_TFT::colorRGB(255, 255, 0)   // Yellow
#define COLOR_RED         DIYables_TFT::colorRGB(255, 0, 0)

// Create TFT display object
DIYables_TFT_GC9A01_Round TFT_display(PIN_RST, PIN_DC, PIN_CS);

// 1 sec = 1000UL;
// 1 min = 60UL*1000UL;
// 1 hour = 60UL*60UL*1000UL;
// 3 minutes = 3UL*60UL*1000UL;
const unsigned long interval = 3UL*60UL*60UL*1000UL;

// Configuration variables (loaded from JSON)
String wifi_ssid = "";
String wifi_password = "";
String ntp_server = "pool.ntp.org";
String timezone = "CET-1CEST,M3.5.0,M10.5.0/3";
unsigned long lastNtp = 0;

// Display settings
#define SCREEN_WIDTH  240
#define SCREEN_HEIGHT 240
#define CENTER_X      120
#define CENTER_Y      120
#define CLOCK_RADIUS  110

// Clock hand lengths
#define HOUR_HAND_LENGTH   50
#define MINUTE_HAND_LENGTH 70
#define SECOND_HAND_LENGTH 85

// Textbox
const int TEXTBOX_WIDTH = 140;
const int TEXTBOX_HEIGHT = 35;
const int TEXTBOX_X = (SCREEN_WIDTH - TEXTBOX_WIDTH) / 2;
const int TEXTBOX_Y = 160;
// Icon
const int ICON_WIDTH = 24;
const int ICON_HEIGHT = 24;
const int ICON_X = CENTER_X - (ICON_WIDTH / 2);
const int ICON_Y = SCREEN_HEIGHT - (TEXTBOX_HEIGHT + TEXTBOX_Y);

enum IconStatus {hide, flash, show};
int iconStatusWifi = IconStatus::show;
bool iconStateWifi = false;
int iconStatusSync = IconStatus::hide;
bool iconStateSync = true;

void setup() {
  // Initialize serial communication
  Serial.begin(115200);
  delay(1000);

  Serial.println("\n\nESP32 WiFi and NTP Time Sync with Clock Display");
  Serial.println("=================================================");

  // Initialize TFT display
  TFT_display.begin();
  TFT_display.setRotation(0);
  TFT_display.fillScreen(COLOR_BACKGROUND);

  // Draw initial clock face
  drawClockFace();
  updateIcons();

  // Initialize LittleFS
  if (!LittleFS.begin(true)) {  // true = format if mount fails
    Serial.println("LittleFS mount failed!");
    TFT_display.setTextColor(COLOR_RED, COLOR_BACKGROUND);
    TFT_display.setTextSize(2);
    TFT_display.setCursor(30, 110);
    TFT_display.print("FS Error!");
    while (1) delay(1000);
  }

  Serial.println("LittleFS mounted successfully!");
  listFiles();

  // Load config or create default
  if (!loadConfig()) {
    Serial.println("Creating default config...");
    createDefaultConfig();

    TFT_display.setTextColor(COLOR_YELLOW, COLOR_BACKGROUND);
    TFT_display.setTextSize(2);
    TFT_display.setCursor(20, 90);
    TFT_display.print("Config File");
    TFT_display.setCursor(30, 110);
    TFT_display.print("Created!");
    TFT_display.setTextSize(1);
    TFT_display.setCursor(10, 140);
    TFT_display.print("Edit config.json");
    TFT_display.setCursor(10, 155);
    TFT_display.print("and restart");

    Serial.println("\n=== IMPORTANT ===");
    Serial.println("Edit config.json file with your WiFi credentials!");
    Serial.println("Then restart the ESP32.");
    Serial.println("=================\n");

    while (1) delay(1000);  // Stop here
  }

  // Check if credentials are still default
  if (wifi_ssid == "YOUR_WIFI_SSID") {
    Serial.println("ERROR: Please edit config.json with your WiFi credentials!");

    TFT_display.setTextColor(COLOR_RED, COLOR_BACKGROUND);
    TFT_display.setTextSize(2);
    TFT_display.setCursor(20, 100);
    TFT_display.print("Edit");
    TFT_display.setCursor(10, 120);
    TFT_display.print("config.json");

    while (1) delay(1000);
  }

  // Connect to WiFi
  // Show "Connecting..." message
  redrawTextBox("Wifi...");
  updateIcons();
  connectToWiFi();

  Serial.println("Syncing time...");

  // Sync time with NTP server
  redrawTextBox("NTP...");
  syncTimeWithNTP();

  Serial.println("Drawing fresh clock face...");

  // Draw clock face on clean screen
  drawTextBox();
  updateClockDisplay();

  Serial.println("Setup complete! Starting clock updates...");
}

void loop() {
  // Update clock display every second
  static unsigned long lastUpdate = 0;
  unsigned long currentMillis = millis();

  if (currentMillis - lastUpdate >= 1000) {
    lastUpdate = currentMillis;
    // Serial.println("Updating display...");
    updateClockDisplay();
    updateIcons();
  }
  if (currentMillis - lastNtp >= interval) {
    syncTimeWithNTP();
  }
}

// Create default config.json file
void createDefaultConfig() {
  Serial.println("Creating default config.json...");

  File file = LittleFS.open("/config.json", "w");
  if (!file) {
    Serial.println("Failed to create config file!");
    return;
  }

  // Create JSON document
  JsonDocument doc;
  doc["wifi_ssid"] = "YOUR_WIFI_SSID";
  doc["wifi_password"] = "YOUR_WIFI_PASSWORD";
  doc["ntp_server"] = "pool.ntp.org";
  doc["timezone"] = "CET-1CEST,M3.5.0,M10.5.0/3";

  // Write to file
  if (serializeJson(doc, file) == 0) {
    Serial.println("Failed to write to config file!");
  } else {
    Serial.println("Default config created successfully!");
  }

  file.close();
}


// Load configuration from JSON file
bool loadConfig() {
  Serial.println("Loading config.json...");

  if (!LittleFS.exists("/config.json")) {
    Serial.println("Config file not found!");
    return false;
  }

  File file = LittleFS.open("/config.json", "r");
  if (!file) {
    Serial.println("Failed to open config file!");
    return false;
  }

  // Parse JSON
  JsonDocument doc;
  DeserializationError error = deserializeJson(doc, file);
  file.close();

  if (error) {
    Serial.println("\n!!! JSON SYNTAX ERROR !!!");
    Serial.print("Parse error: ");
    Serial.println(error.c_str());

    // Show the actual file contents
    file = LittleFS.open("/config.json", "r");
    if (file) {
      Serial.println("\n=== Current config.json contents ===");
      while (file.available()) {
        Serial.write(file.read());
      }
      file.close();
      Serial.println("\n====================================\n");
    }

    Serial.println("The config.json file has invalid JSON syntax.");
    Serial.println("Common issues:");
    Serial.println("  - Missing comma between fields");
    Serial.println("  - Missing quotes around strings");
    Serial.println("  - Trailing comma after last field");
    Serial.println("  - Unescaped special characters");
    Serial.println("\nPlease fix config.json and restart.");

    // Show error on display
    TFT_display.fillScreen(COLOR_BACKGROUND);
    TFT_display.setTextColor(COLOR_RED, COLOR_BACKGROUND);
    TFT_display.setTextSize(2);
    TFT_display.setCursor(30, 90);
    TFT_display.print("JSON Error!");
    TFT_display.setTextSize(1);
    TFT_display.setCursor(10, 120);
    TFT_display.print("Syntax error in");
    TFT_display.setCursor(10, 135);
    TFT_display.print("config.json");
    TFT_display.setCursor(10, 155);
    TFT_display.print("Check Serial Monitor");
    TFT_display.setCursor(10, 170);
    TFT_display.print("for details");

    // Stop here - don't overwrite the file!
    while (1) delay(1000);
  }

  // Load values
  wifi_ssid = doc["wifi_ssid"].as<String>();
  wifi_password = doc["wifi_password"].as<String>();
  ntp_server = doc["ntp_server"] | "pool.ntp.org";
  timezone = doc["timezone"] | "CET-1CEST,M3.5.0,M10.5.0/3";

  // Validate required fields
  if (wifi_ssid.isEmpty()) {
    Serial.println("\n!!! VALIDATION ERROR !!!");
    Serial.println("Missing 'wifi_ssid' field in config.json");

    TFT_display.fillScreen(COLOR_BACKGROUND);
    TFT_display.setTextColor(COLOR_RED, COLOR_BACKGROUND);
    TFT_display.setTextSize(2);
    TFT_display.setCursor(20, 100);
    TFT_display.print("Missing SSID");
    TFT_display.setTextSize(1);
    TFT_display.setCursor(10, 130);
    TFT_display.print("in config.json");

    while (1) delay(1000);
  }

  if (wifi_password.isEmpty()) {
    Serial.println("\n!!! VALIDATION ERROR !!!");
    Serial.println("Missing 'wifi_password' field in config.json");

    TFT_display.fillScreen(COLOR_BACKGROUND);
    TFT_display.setTextColor(COLOR_RED, COLOR_BACKGROUND);
    TFT_display.setTextSize(2);
    TFT_display.setCursor(10, 100);
    TFT_display.print("Missing PWD");
    TFT_display.setTextSize(1);
    TFT_display.setCursor(10, 130);
    TFT_display.print("in config.json");

    while (1) delay(1000);
  }

  Serial.println("Config loaded successfully!");
  Serial.println("SSID: " + wifi_ssid);
  Serial.println("NTP Server: " + ntp_server);
  Serial.println("Timezone: " + timezone);

  return true;
}

// List all files in LittleFS (for debugging)
void listFiles() {
  Serial.println("\n=== Files in LittleFS ===");
  File root = LittleFS.open("/");
  File file = root.openNextFile();

  while (file) {
    Serial.print("  ");
    Serial.print(file.name());
    Serial.print(" (");
    Serial.print(file.size());
    Serial.println(" bytes)");
    file = root.openNextFile();
  }
  Serial.println("========================\n");
}

// Show contents of config.json (for debugging)
void showConfigFile() {
  Serial.println("\n=== config.json contents ===");
  File file = LittleFS.open("/config.json", "r");
  if (file) {
    while (file.available()) {
      Serial.write(file.read());
    }
    file.close();
    Serial.println("\n============================\n");
  } else {
    Serial.println("Could not open config.json");
  }
}

void connectToWiFi() {
  iconStatusWifi = IconStatus::flash;
  Serial.print("Connecting to WiFi: ");
  Serial.println(wifi_ssid);
  // Start WiFi connection
  WiFi.begin(wifi_ssid.c_str(), wifi_password.c_str());

  // Wait for connection
  int attempts = 0;
  while (WiFi.status() != WL_CONNECTED && attempts < 30) {
    delay(500);
    Serial.print(".");
    iconStatusWifi = attempts % 2 == 0 ? IconStatus::hide : IconStatus::flash;
    updateWifiIcon();
    attempts++;
  }

  if (WiFi.status() == WL_CONNECTED) {
    Serial.println("\nWiFi connected successfully!");
    Serial.print("IP Address: ");
    Serial.println(WiFi.localIP());
    Serial.print("Signal Strength (RSSI): ");
    Serial.print(WiFi.RSSI());
    Serial.println(" dBm");
  } else {
    Serial.println("\nFailed to connect to WiFi!");
    Serial.println("Please check your credentials and try again.");
  }
  iconStatusWifi = IconStatus::show;
}

void syncTimeWithNTP() {
  if (WiFi.status() == WL_CONNECTED) {
    iconStatusSync = IconStatus::flash;
    Serial.println("\nSynchronizing time with NTP server...");

    // Configure time with NTP server using timezone string
    configTzTime(timezone.c_str(), ntp_server.c_str());

    // Wait for time to be set
    struct tm timeinfo;
    int attempts = 0;
    while (!getLocalTime(&timeinfo) && attempts < 10) {
      Serial.print(".");
      delay(1000);
      iconStatusSync = attempts % 2 == 0 ? IconStatus::hide : IconStatus::flash;
      updateSyncIcon();
      attempts++;
    }

    if (getLocalTime(&timeinfo)) {
      Serial.println("\nTime synchronized successfully!");
      Serial.print("Timezone: ");
      Serial.println(timezone.c_str());
      lastNtp = millis();
    } else {
      Serial.println("\nFailed to obtain time from NTP server!");
    }
  } else {
    writeText("Sync error", true);
    Serial.println("Cannot sync time - WiFi not connected!");
  }
  iconStatusSync = IconStatus::hide;
}

void drawClockFace() {
  // Draw outer circle
  TFT_display.drawCircle(CENTER_X, CENTER_Y, CLOCK_RADIUS, COLOR_CLOCKFACE);
  TFT_display.drawCircle(CENTER_X, CENTER_Y, CLOCK_RADIUS - 1, COLOR_CLOCKFACE);

  // Draw hour markers
  for (int i = 0; i < 12; i++) {
    float angle = i * 30 - 90;  // -90 to start from top (12 o'clock)
    float angleRad = angle * PI / 180.0;

    // Outer point
    int x1 = CENTER_X + (CLOCK_RADIUS - 5) * cos(angleRad);
    int y1 = CENTER_Y + (CLOCK_RADIUS - 5) * sin(angleRad);

    // Inner point
    int x2 = CENTER_X + (CLOCK_RADIUS - 15) * cos(angleRad);
    int y2 = CENTER_Y + (CLOCK_RADIUS - 15) * sin(angleRad);

    // Draw thicker lines for 12, 3, 6, 9
    if (i % 3 == 0) {
      TFT_display.drawLine(x1, y1, x2, y2, COLOR_CLOCKFACE);
      TFT_display.drawLine(x1 + 1, y1, x2 + 1, y2, COLOR_CLOCKFACE);
      TFT_display.drawLine(x1, y1 + 1, x2, y2 + 1, COLOR_CLOCKFACE);
    } else {
      TFT_display.drawLine(x1, y1, x2, y2, COLOR_CLOCKFACE);
    }
  }

  // Draw center dot
  TFT_display.fillCircle(CENTER_X, CENTER_Y, 4, COLOR_CENTER_DOT);
  drawTextBox();
}

void drawTextBox() {
  // Draw display box.
  TFT_display.fillRect(TEXTBOX_X, TEXTBOX_Y, TEXTBOX_WIDTH, TEXTBOX_HEIGHT, COLOR_BACKGROUND);
  TFT_display.drawRect(TEXTBOX_X, TEXTBOX_Y, TEXTBOX_WIDTH, TEXTBOX_HEIGHT, COLOR_CLOCKFACE);
}

void redrawTextBox(const char str[]) {
  drawTextBox();
  writeText(str, false);
}

void writeText(const char str[], bool center) {
  TFT_display.setTextColor(COLOR_YELLOW, COLOR_BACKGROUND);
  TFT_display.setTextSize(2);

  int x = TEXTBOX_X + 5;
  if (center) {
    int textWidth = strlen(str) * 12;  // Approximate width
    x = (SCREEN_WIDTH - textWidth) / 2;
  }

  TFT_display.setCursor(x, 170);
  TFT_display.print(str);
}

void updateClockDisplay() {
  struct tm timeinfo;
  if (!getLocalTime(&timeinfo)) {
    Serial.println("ERROR: Failed to get local time!");
    return;
  }

  // Serial.printf("Time: %02d:%02d:%02d\n", timeinfo.tm_hour, timeinfo.tm_min, timeinfo.tm_sec);

  // Display digital time at bottom
  displayDigitalTime(timeinfo);
  static int lastSecond = -1;
  static int lastMinute = -1;
  static int lastHour = -1;
/*
  int currentSecond = timeinfo.tm_sec;
  int currentMinute = timeinfo.tm_min;
  int currentHour = timeinfo.tm_hour % 12;

  // Erase old hands by redrawing them in background color
  if (lastSecond != -1) {
    drawSecondHand(lastSecond, COLOR_BACKGROUND);
  }
  if (lastMinute != -1) {
    drawMinuteHand(lastMinute, lastHour, COLOR_BACKGROUND);
  }
  if (lastHour != -1) {
    drawHourHand(lastHour, lastMinute, COLOR_BACKGROUND);
  }

  // Draw new hands
  drawHourHand(currentHour, currentMinute, COLOR_HOUR_HAND);
  drawMinuteHand(currentMinute, currentHour, COLOR_MINUTE_HAND);
  drawSecondHand(currentSecond, COLOR_SECOND_HAND);

  // Redraw center dot on top
  TFT_display.fillCircle(CENTER_X, CENTER_Y, 4, COLOR_CENTER_DOT);

  // Update last positions
  lastSecond = currentSecond;
  lastMinute = currentMinute;
  lastHour = currentHour;

  // Display digital time at bottom
  displayDigitalTime(timeinfo);

  Serial.println("Display updated successfully");
*/
// Add preset offset to millis()
  //unsigned long ms = millis() + PRESET_MS;

  //float sec = fmod(ms / 1000.0, 60.0);
  //float min = fmod(ms / 60000.0, 60.0);
  //float hour = fmod(ms / 3600000.0, 12.0);

  int sec = timeinfo.tm_sec;
  int min = timeinfo.tm_min;
  int hour = timeinfo.tm_hour % 12;

  int dispHour = (int)hour == 0 ? 12 : (int)hour;
  int dispMin = (int)min;
  int dispSec = (int)sec;

  float hourAngle = (hour + min / 60.0) * 30 * M_PI / 180.0;
  float minAngle = (min + sec / 60.0) * 6 * M_PI / 180.0;
  float secAngle = sec * 6 * M_PI / 180.0;
/*
  // Hour hand (jumps), redraw when angle changed
  if (dispHour != prevDispHour || dispMin != prevDispMin) {
    if (prevHourAngle > -900)
      drawHand(CENTER_X, CENTER_Y, prevHourAngle, HOUR_LEN, COLOR_BACKGROUND, 7);  // clear old position
    drawHand(CENTER_X, CENTER_Y, hourAngle, HOUR_LEN, COLOR_HOUR, 7);
    prevHourAngle = hourAngle;

    // Update digital display
    if (dispHour != prevDispHour) {
      printTime(prevDispHour, dispHour, "");
      prevDispHour = dispHour;
    }
  }

  // Minute hand (jumps)
  if (dispMin != prevDispMin) {
    if (prevMinAngle > -900)
      drawHand(CENTER_X, CENTER_Y, prevMinAngle, MIN_LEN, COLOR_BACKGROUND, 5);  // clear old position
    drawHand(CENTER_X, CENTER_Y, minAngle, MIN_LEN, COLOR_MINUTE, 5);

    printTime(prevDispMin, dispMin, "00:");
    prevDispMin = dispMin;
    prevMinAngle = minAngle;
  }

  // Second hand (smooth)
  if (dispSec != prevDispSec) {
    if (prevSecAngle > -900)
      drawHand(CENTER_X, CENTER_Y, prevSecAngle, SEC_LEN, COLOR_BACKGROUND, 2);  // clear old position

    drawHand(CENTER_X, CENTER_Y, secAngle, SEC_LEN, COLOR_SECOND, 2);
    TFT_display.fillCircle(CENTER_X, CENTER_Y, 7, COLOR_SECOND);  // Redraw center dot

    // Update digital display
    printTime(prevDispSec, dispSec, "00:00:", false);
    prevDispSec = dispSec;
    prevSecAngle = secAngle;
  }
*/
}

void drawHourHand(int hour, int minute, uint16_t color) {
  // Hour hand moves gradually with minutes
  float angle = (hour * 30 + minute * 0.5) - 90;  // -90 to start from top
  float angleRad = angle * PI / 180.0;

  int x = CENTER_X + HOUR_HAND_LENGTH * cos(angleRad);
  int y = CENTER_Y + HOUR_HAND_LENGTH * sin(angleRad);

  // Draw thick line
  TFT_display.drawLine(CENTER_X, CENTER_Y, x, y, color);
  TFT_display.drawLine(CENTER_X + 1, CENTER_Y, x + 1, y, color);
  TFT_display.drawLine(CENTER_X, CENTER_Y + 1, x, y + 1, color);
}

void drawMinuteHand(int minute, int hour, uint16_t color) {
  float angle = (minute * 6) - 90;  // -90 to start from top
  float angleRad = angle * PI / 180.0;

  int x = CENTER_X + MINUTE_HAND_LENGTH * cos(angleRad);
  int y = CENTER_Y + MINUTE_HAND_LENGTH * sin(angleRad);

  // Draw medium thick line
  TFT_display.drawLine(CENTER_X, CENTER_Y, x, y, color);
  TFT_display.drawLine(CENTER_X + 1, CENTER_Y, x + 1, y, color);
}

void drawSecondHand(int second, uint16_t color) {
  float angle = (second * 6) - 90;  // -90 to start from top
  float angleRad = angle * PI / 180.0;

  int x = CENTER_X + SECOND_HAND_LENGTH * cos(angleRad);
  int y = CENTER_Y + SECOND_HAND_LENGTH * sin(angleRad);

  // Draw thin line
  TFT_display.drawLine(CENTER_X, CENTER_Y, x, y, color);
}

void displayDigitalTime(struct tm &timeinfo) {
  // Clear previous time display area

  // Format time string
  char timeStr[16];
  sprintf(timeStr, "%02d:%02d:%02d", timeinfo.tm_hour, timeinfo.tm_min, timeinfo.tm_sec);
  writeText(timeStr, true);
}

// Icons.
void updateIcons() {
  updateWifiIcon();
  updateSyncIcon();
}

void updateWifiIcon() {
  bool visible = false;
  bool okIcon = true;

  if (iconStatusWifi == IconStatus::show) {
    okIcon = WiFi.status() == WL_CONNECTED;
    visible = true;
  }
  else if (iconStatusWifi == IconStatus::flash) {
    visible = !iconStateWifi;
    iconStateWifi = !iconStateWifi;
    okIcon = true;
  }
  else {
    visible = false;
  }

  drawIcon(visible, CENTER_X - 2 - ICON_WIDTH, okIcon ? IconWifiBitmap : IconWifiOffBitmap);
}

void updateSyncIcon() {
  bool visible = false;
  if (iconStatusSync == IconStatus::show) {
    visible = true;
  }
  else if (iconStatusSync == IconStatus::flash) {
    iconStateWifi = !iconStateSync;
    visible = iconStateSync;
  }
  else {
    visible = false;
  }

  drawIcon(visible, CENTER_X + 2, IconSyncBitmap);
}

void drawIcon(bool visible, int x, const uint16_t* icon) {
  if (visible) {
    TFT_display.drawRGBBitmap(x, ICON_Y, icon, ICON_WIDTH, ICON_HEIGHT);
  }
  else {
    TFT_display.fillRect(x, ICON_Y, ICON_WIDTH, ICON_HEIGHT, COLOR_BACKGROUND);
  }
}
