#include <WiFi.h>
#include <time.h>
#include <DIYables_TFT_Round.h>
#include "config.h"  // Include your config file

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

// Create TFT display object
DIYables_TFT_GC9A01_Round TFT_display(PIN_RST, PIN_DC, PIN_CS);

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

  // Show "Connecting..." message
  TFT_display.setTextColor(COLOR_YELLOW);
  TFT_display.setTextSize(2);
  TFT_display.setCursor(50, 110);
  TFT_display.print("Connecting");
  TFT_display.setCursor(60, 130);
  TFT_display.print("WiFi...");

  // Connect to WiFi
  connectToWiFi();

  Serial.println("Clearing screen completely...");

  // Clear the entire screen (not just a circle)
  TFT_display.fillScreen(COLOR_BACKGROUND);

  Serial.println("Syncing time...");

  // Sync time with NTP server
  syncTimeWithNTP();

  Serial.println("Drawing fresh clock face...");

  // Draw clock face on clean screen
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
  }
  if (currentMillis - lastNtp >= interval) {
    syncTimeWithNTP();
  }
}

void connectToWiFi() {
  Serial.print("Connecting to WiFi: ");
  Serial.println(ssid);

  // Start WiFi connection
  WiFi.begin(ssid, password);

  // Wait for connection
  int attempts = 0;
  while (WiFi.status() != WL_CONNECTED && attempts < 30) {
    delay(500);
    Serial.print(".");
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
}

void syncTimeWithNTP() {
  if (WiFi.status() == WL_CONNECTED) {
    Serial.println("\nSynchronizing time with NTP server...");

    // Configure time with NTP server using timezone string
    configTzTime(timeZone, ntpServer);

    // Wait for time to be set
    struct tm timeinfo;
    int attempts = 0;
    while (!getLocalTime(&timeinfo) && attempts < 10) {
      Serial.print(".");
      delay(1000);
      attempts++;
    }

    if (getLocalTime(&timeinfo)) {
      Serial.println("\nTime synchronized successfully!");
      Serial.print("Timezone: ");
      Serial.println(timeZone);
      lastNtp = millis();
    } else {
      Serial.println("\nFailed to obtain time from NTP server!");
    }
  } else {
    Serial.println("Cannot sync time - WiFi not connected!");
  }
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

  // Draw display box.
  TFT_display.drawRect(40, 160, 160, 50, COLOR_CLOCKFACE);
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
  // TFT_display.fillRect(0, 200, 240, 40, COLOR_BACKGROUND);

  // Format time string
  char timeStr[16];
  sprintf(timeStr, "%02d:%02d:%02d", timeinfo.tm_hour, timeinfo.tm_min, timeinfo.tm_sec);

  // Display time
  TFT_display.setTextColor(COLOR_YELLOW, COLOR_BACKGROUND);
  TFT_display.setTextSize(2);

  // Center the text
  int textWidth = strlen(timeStr) * 12;  // Approximate width
  int x = (SCREEN_WIDTH - textWidth) / 2;
  TFT_display.setCursor(x, 170);
  TFT_display.print(timeStr);
}
