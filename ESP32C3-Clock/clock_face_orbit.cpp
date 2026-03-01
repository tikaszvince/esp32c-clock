#include <time.h>
#include <math.h>
#include <ctype.h>
#include "clock_face_orbit.h"
#include "display.h"
#include "display_constants.h"
#include "clock_face_helpers.h"
#include "icons.h"

// Colors local to this face
static const uint16_t COLOR_ORBIT_TRACK = DIYables_TFT::colorRGB(50, 50, 50);
static const uint16_t COLOR_ORBIT_DATE = DIYables_TFT::colorRGB(120, 120, 120);
static const uint16_t COLOR_ORBIT_WEEKEND = DIYables_TFT::colorRGB(255, 140, 0);
static const uint16_t COLOR_ARC_YEAR = DIYables_TFT::colorRGB(255, 180, 0);
static const uint16_t COLOR_ARC_MONTH = DIYables_TFT::colorRGB(50, 220, 120);
static const uint16_t COLOR_ARC_DAY = DIYables_TFT::colorRGB(0, 180, 255);
static const uint16_t COLOR_ARC_MINUTE = DIYables_TFT::colorRGB(200, 80, 255);

// Arc geometry.
static const int ARC_THICKNESS = 3;
static const int ARC_PADDING = 2;
static const int ARC_STRIDE = ARC_THICKNESS + ARC_PADDING;
static const int ARC_YEAR_OUTER = 119;
static const int ARC_YEAR_INNER = ARC_YEAR_OUTER - (ARC_THICKNESS - 1);
static const int ARC_MONTH_OUTER = ARC_YEAR_INNER - ARC_PADDING - 1;
static const int ARC_MONTH_INNER = ARC_MONTH_OUTER - (ARC_THICKNESS - 1);
static const int ARC_DAY_OUTER = ARC_MONTH_INNER - ARC_PADDING - 1;
static const int ARC_DAY_INNER = ARC_DAY_OUTER - (ARC_THICKNESS - 1);
static const int ARC_MINUTE_OUTER = ARC_DAY_INNER - ARC_PADDING - 1;
static const int ARC_MINUTE_INNER = ARC_MINUTE_OUTER - (ARC_THICKNESS - 1);
static const float ARC_STEP_DEG = 0.3f;

// Layout — total content block (time + gap + date + day name + week segment)
// centered at y=120, so block starts at y=87
static const int TIME_Y = 72;
static const int TIME_CHAR_W = 30;
static const int TIME_CHAR_H = 40;
static const int DATE_Y = (TIME_Y + TIME_CHAR_H + 12);
static const int DATE_CHAR_W = 12;
static const int DATE_CHAR_H = 16;
static const int DAY_NAME_Y = (DATE_Y + DATE_CHAR_H + 6);
static const int SEG_Y = (DAY_NAME_Y + DATE_CHAR_H + 8);
static const int SEG_W = 12;
static const int SEG_H = 6;
static const int SEG_GAP = 2;
static const int SEG_COUNT = 7;
static const int SEG_TOTAL_W = SEG_COUNT * SEG_W + (SEG_COUNT - 1) * SEG_GAP;
static const int SEG_START_X = (SCREEN_WIDTH - SEG_TOTAL_W) / 2;

// Icons positions.
static const int ICON_SIZE = 24;
static const int ICON_PADDING = 26;
static const int ICON_WIFI_X = (SCREEN_WIDTH / 2) - (ICON_SIZE / 2);
// Center bottom with padding from bottom
static const int ICON_WIFI_Y = (SCREEN_HEIGHT - ICON_SIZE - ICON_PADDING);
static const int ICON_NTP_X = (SCREEN_WIDTH / 2) - (ICON_SIZE / 2);
// Center top with padding from top
static const int ICON_NTP_Y = ICON_PADDING;

void ClockFaceOrbit::reset() {
  _needsFullRedraw = true;
}

void ClockFaceOrbit::draw(AppState state, bool blinkState) {
  if (_needsFullRedraw) {
    drawBackground();
    drawIcons(state, blinkState);
    _lastState = state;
    _lastBlinkState = blinkState;
    _lastMinute = -2;
    _lastDay = -2;
    _needsFullRedraw = false;
  }

  struct tm timeinfo;
  bool timeValid = getLocalTime(&timeinfo);
  if (timeValid) {
    _lastValidHour = timeinfo.tm_hour;
    _lastValidMinute = timeinfo.tm_min;
  }

  int displayHour = _lastValidHour;
  int displayMinute = _lastValidMinute;

  bool minuteChanged = (displayMinute != _lastMinute);
  bool dayChanged = timeValid && (timeinfo.tm_mday != _lastDay);
  bool iconsChanged = (state != _lastState || blinkState != _lastBlinkState);

  if (minuteChanged) {
    drawArcTrack(timeValid ? &timeinfo : nullptr, displayMinute);
    drawTime(displayHour, displayMinute);
    _lastMinute = displayMinute;
  }

  if (dayChanged) {
    drawDate(&timeinfo);
    _lastDay = timeinfo.tm_mday;
  }

  if (iconsChanged) {
    drawIcons(state, blinkState);
    _lastState = state;
    _lastBlinkState = blinkState;
  }
}

void ClockFaceOrbit::drawBackground() {
  TFT_display.fillScreen(COLOR_BACKGROUND);
}

static bool isLeapYear(int year) {
  return (year % 4 == 0 && year % 100 != 0) || (year % 400 == 0);
}

static int daysInMonth(int month, int year) {
  static const int days[] = {31, 28, 31, 30, 31, 30, 31, 31, 30, 31, 30, 31};
  if (month == 1 && isLeapYear(year)) {
    return 29;
  }
  return days[month];
}

static void drawSingleArc(float fraction, int innerR, int outerR, uint16_t arcColor) {
  float filledDeg = fraction * 360.0f;
  for (float angle = 0.0f; angle < 360.0f; angle += ARC_STEP_DEG) {
    float rad = (angle - 90.0f) * PI / 180.0f;
    uint16_t color = (angle < filledDeg) ? arcColor : COLOR_ORBIT_TRACK;
    for (int r = innerR; r <= outerR; r++) {
      int x = CENTER_X + (int)roundf(r * cosf(rad));
      int y = CENTER_Y + (int)roundf(r * sinf(rad));
      TFT_display.drawPixel(x, y, color);
    }
  }
}

void ClockFaceOrbit::drawArcTrack(const struct tm* timeinfo, int displayMinute) {
  float minuteFraction = (displayMinute < 0) ? 0.0f : displayMinute / 60.0f;
  drawSingleArc(minuteFraction, ARC_MINUTE_INNER, ARC_MINUTE_OUTER, COLOR_ARC_MINUTE);

  if (timeinfo == nullptr) {
    return;
  }

  float dayFraction = (timeinfo->tm_hour * 60 + timeinfo->tm_min + 1) / 1440.0f;
  drawSingleArc(dayFraction, ARC_DAY_INNER, ARC_DAY_OUTER, COLOR_ARC_DAY);

  int totalDaysInMonth = daysInMonth(timeinfo->tm_mon, timeinfo->tm_year + 1900);
  float monthFraction = timeinfo->tm_mday / (float)totalDaysInMonth;
  drawSingleArc(monthFraction, ARC_MONTH_INNER, ARC_MONTH_OUTER, COLOR_ARC_MONTH);

  int totalDaysInYear = isLeapYear(timeinfo->tm_year + 1900) ? 366 : 365;
  float yearFraction = (timeinfo->tm_yday + 1) / (float)totalDaysInYear;
  drawSingleArc(yearFraction, ARC_YEAR_INNER, ARC_YEAR_OUTER, COLOR_ARC_YEAR);
}

void ClockFaceOrbit::drawTime(int hour, int minute) {
  char buf[8];
  if (hour < 0) {
    strcpy(buf, "--:--");
  }
  else {
    sprintf(buf, "%02d:%02d", hour, minute);
  }
  int w = strlen(buf) * TIME_CHAR_W;
  int x = (SCREEN_WIDTH - w) / 2;
  // TFT_display.fillRect(x, TIME_Y, w, TIME_CHAR_H, COLOR_BACKGROUND);
  TFT_display.setTextSize(5);
  TFT_display.setTextColor(COLOR_CLOCKFACE, COLOR_BACKGROUND);
  TFT_display.setCursor(x, TIME_Y);
  TFT_display.print(buf);
}

void ClockFaceOrbit::drawDate(const struct tm* timeinfo) {
  // Line 1: YYYY-MM-DD
  char dateBuf[12];
  strftime(dateBuf, sizeof(dateBuf), "%Y-%m-%d", timeinfo);
  int dateW = strlen(dateBuf) * DATE_CHAR_W;
  int dateX = (SCREEN_WIDTH - dateW) / 2;
  // TFT_display.fillRect(dateX, DATE_Y, dateW, DATE_CHAR_H, COLOR_BACKGROUND);
  TFT_display.setTextSize(2);
  TFT_display.setTextColor(COLOR_ORBIT_DATE, COLOR_BACKGROUND);
  TFT_display.setCursor(dateX, DATE_Y);
  TFT_display.print(dateBuf);

  // Line 2: day name
  char dayBuf[12];
  strftime(dayBuf, sizeof(dayBuf), "%A", timeinfo);
  for (int i = 0; dayBuf[i]; i++) {
    dayBuf[i] = toupper((unsigned char)dayBuf[i]);
  }
  int dayW = strlen(dayBuf) * DATE_CHAR_W;
  int dayX = (SCREEN_WIDTH - dayW) / 2;
  // TFT_display.fillRect(dayX, DAY_NAME_Y, dayW, DATE_CHAR_H, COLOR_BACKGROUND);
  TFT_display.setTextColor(COLOR_ORBIT_DATE, COLOR_BACKGROUND);
  TFT_display.setCursor(dayX, DAY_NAME_Y);
  TFT_display.print(dayBuf);

  // 7-segment day indicator
  int dow = (timeinfo->tm_wday + 6) % 7;
  drawDaySegments(dow);
}

void ClockFaceOrbit::drawDaySegments(int dayOfWeek) {
  for (int i = 0; i < SEG_COUNT; i++) {
    int x = SEG_START_X + i * (SEG_W + SEG_GAP);
    bool isActive = (i == dayOfWeek);
    bool isWeekend = (i >= 5);
    uint16_t color;
    if (isActive) {
      color = isWeekend ? COLOR_ORBIT_WEEKEND : COLOR_ARC_MINUTE;
    }
    else {
      color = COLOR_ORBIT_TRACK;
    }
    TFT_display.fillRect(x, SEG_Y, SEG_W, SEG_H, color);
  }
}

void ClockFaceOrbit::drawIcons(AppState state, bool blinkState) {
  drawStatusIcons(state, blinkState, ICON_WIFI_X, ICON_WIFI_Y, ICON_NTP_X, ICON_NTP_Y);
}
