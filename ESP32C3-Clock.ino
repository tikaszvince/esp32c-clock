#include <WiFi.h>
#include <time.h>
#include <DIYables_TFT_Round.h>
#include <math.h>

#include "icons.h"
#include "config.h"  // WiFiManager configuration

// TFT Display pins.
#define PIN_RST 27  // The ESP32 pin GPIO27 connected to the RST pin of the circular TFT display
#define PIN_DC  25  // The ESP32 pin GPIO25 connected to the DC pin of the circular TFT display
#define PIN_CS  26  // The ESP32 pin GPIO26 connected to the CS pin of the circular TFT display

// Colors
#define COLOR_BACKGROUND  DIYables_TFT::colorRGB(0, 0, 0)       // Black
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

// Variables will change:
//int lastStateBtn = LOW;  // the previous state from the input pin
//int currentStateBtn;     // the current reading from the input pin
//unsigned long pressedTimeBtn  = 0;
//unsigned long releasedTimeBtn = 0;

//int lastStateBoot = LOW;  // the previous state from the input pin
//int currentStateBoot;     // the current reading from the input pin
//unsigned long pressedTimeBoot = 0;
//unsigned long releasedTimeBoot = 0;
//bool isPressingBoot = false;


// Setup a new OneButton on pin PIN_INPUT2.
OneButton buttonBoot(BOOT_BUTTON_PIN, true);

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

  // Show WiFi setup message
  redrawTextBox("WiFi Setup");
  iconStatusWifi = IconStatus::flash;
  updateIcons();

  // Initialize WiFi configuration (web portal)
  if (!initConfig()) {
    // Failed to connect
    redrawTextBox("WiFi Failed");
    iconStatusWifi = IconStatus::show;
    updateIcons();
    delay(3000);
    ESP.restart();
  }

  iconStatusWifi = IconStatus::show;
  updateIcons();

  // Sync time with NTP server
  redrawTextBox("NTP Sync...");
  syncTimeWithNTP();

  // Draw clock display
  drawTextBox();
  updateClockDisplay();

  Serial.println("Setup complete!");
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

void syncTimeWithNTP() {
  iconStatusSync = IconStatus::flash;

  if (WiFi.status() == WL_CONNECTED) {
    Serial.println("\nSynchronizing time with NTP server...");

    // List of NTP servers to try
    const char* ntpServerList[] = {
      getNTPServer().c_str(),
      "pool.ntp.org",
      "time.google.com",
      "time.cloudflare.com",
      "time.windows.com",
      "hu.pool.ntp.org"
    };

    bool timeSet = false;

    // Try each server (skip empty ones)
    for (int serverIdx = 0; serverIdx < 6 && !timeSet; serverIdx++) {
      const char* server = ntpServerList[serverIdx];

      // Skip empty or null servers
      if (server == NULL || strlen(server) == 0) {
        Serial.println("Skipping empty NTP server");
        continue;
      }

      Serial.print("Trying NTP server: ");
      Serial.println(server);

      configTzTime(getTimezone().c_str(), server);

      // Wait for time sync
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
        Serial.printf("Time: %02d:%02d:%02d\n",
                      timeinfo.tm_hour, timeinfo.tm_min, timeinfo.tm_sec);
        timeSet = true;
        lastNtp = millis();
        writeText("Time synced", true);
        break;
      } else {
        Serial.println(" Failed!");
      }
    }

    if (!timeSet) {
      writeText("Sync failed", true);
      Serial.println("Failed with all NTP servers!");
    }
  } else {
    writeText("No WiFi", true);
    Serial.println("Cannot sync - WiFi not connected!");
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
    yield();  // Prevent watchdog
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

  // Display digital time
  displayDigitalTime(timeinfo);
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
    iconStateSync = !iconStateSync;
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
