#include <math.h>
#include <time.h>
#include "clock_face_bauhaus.h"
#include "display.h"
#include "display_constants.h"
#include "clock_face_helpers.h"
#include "app_state.h"

// Face geometry
static const int FACE_RING_RADIUS = 115;
static const int MARKER_RADIUS = 108;
static const int MARKER_MAJOR_SIZE = 8;
static const int MARKER_MINOR_SIZE = 3;
static const int CENTER_CLIP_RADIUS = 6;

// Hands
static const int HOUR_HAND_LENGTH = 55;
static const int HOUR_HAND_WIDTH = 6;
static const int MINUTE_HAND_LENGTH = 95;
static const int MINUTE_HAND_WIDTH = 4;

// Counterweight
static const int COUNTERWEIGHT_DIST = 15;
static const int COUNTERWEIGHT_RADIUS = 5;
static const int COUNTERWEIGHT_CLIP_R = 90;

// Digital time (3 o'clock position)
static const int TIME_CHAR_W = 18;  // textSize 3: 6 * 3
static const int TIME_CHAR_H = 24;  // textSize 3: 8 * 3
static const int TIME_TEXT_W = 5 * TIME_CHAR_W;
static const int TIME_TEXT_PADDING    = 8;
static const int TIME_TEXT_X = SCREEN_WIDTH - TIME_TEXT_W - TIME_TEXT_PADDING;
static const int TIME_TEXT_Y = CENTER_Y - (TIME_CHAR_H / 2);

// Status dot (6 o'clock position, replaces major marker there)
static const int STATUS_DOT_X = CENTER_X;
static const int STATUS_DOT_Y = CENTER_Y + MARKER_RADIUS;
static const int STATUS_DOT_RADIUS = MARKER_MAJOR_SIZE;

static bool isClippedBauhaus(int x, int y) {
  int dx = x - CENTER_X;
  int dy = y - CENTER_Y;
  if (dx * dx + dy * dy <= CENTER_CLIP_RADIUS * CENTER_CLIP_RADIUS) {
    return true;
  }
  if (
    x >= TIME_TEXT_X && x < TIME_TEXT_X + TIME_TEXT_W
    && y >= TIME_TEXT_Y && y < TIME_TEXT_Y + TIME_CHAR_H
  ) {
    return true;
  }
  return false;
}

ClockFaceBauhaus::ClockFaceBauhaus(const BauhausTheme& theme)
  : _theme(theme) {
}

ClockFaceBauhaus ClockFaceBauhaus::createLight() {
  BauhausTheme t;
  t.background = DIYables_TFT::colorRGB(255, 255, 255);
  t.face = DIYables_TFT::colorRGB(0, 0, 0);
  t.handHour = DIYables_TFT::colorRGB(0, 0, 0);
  t.handMinute = DIYables_TFT::colorRGB(0, 0, 0);
  t.counterweight = DIYables_TFT::colorRGB(210, 30, 30);
  t.markerMajor = DIYables_TFT::colorRGB(0, 0, 0);
  t.markerMinor = DIYables_TFT::colorRGB(0, 0, 0);
  t.statusOk = DIYables_TFT::colorRGB(30, 160, 60);
  t.statusNoWifi = DIYables_TFT::colorRGB(210, 30, 30);
  t.statusSyncing = DIYables_TFT::colorRGB(30, 90, 210);
  return ClockFaceBauhaus(t);
}

ClockFaceBauhaus ClockFaceBauhaus::createDark() {
  BauhausTheme t;
  t.background = DIYables_TFT::colorRGB(0, 0, 0);
  t.face = DIYables_TFT::colorRGB(200, 200, 200);
  t.handHour = DIYables_TFT::colorRGB(200, 200, 200);
  t.handMinute = DIYables_TFT::colorRGB(200, 200, 200);
  t.counterweight = DIYables_TFT::colorRGB(240, 50, 50);
  t.markerMajor = DIYables_TFT::colorRGB(200, 200, 200);
  t.markerMinor = DIYables_TFT::colorRGB(120, 120, 120);
  t.statusOk = DIYables_TFT::colorRGB(40, 200, 80);
  t.statusNoWifi = DIYables_TFT::colorRGB(240, 50, 50);
  t.statusSyncing = DIYables_TFT::colorRGB(50, 120, 240);
  return ClockFaceBauhaus(t);
}

void ClockFaceBauhaus::reset() {
  _needsFullRedraw = true;
}

void ClockFaceBauhaus::draw(AppState state, bool blinkState) {
  if (_needsFullRedraw) {
    drawBackground();
    drawFaceRing();
    _lastHourAngle = -1.0f;
    _lastMinuteAngle = -1.0f;
    _lastStatusColor = 0;
    _lastTimeText[0] = '\0';
    _lastHourPixelCount = 0;
    _lastMinutePixelCount = 0;
    _lastCounterweightValid = false;
    _needsFullRedraw = false;
  }

  struct tm timeinfo;
  if (getLocalTime(&timeinfo)) {
    float hourAngle = roundAngle((timeinfo.tm_hour % 12) * 30.0f + timeinfo.tm_min * 0.5f);
    float minuteAngle = roundAngle(timeinfo.tm_min * 6.0f);

    if (hourAngle != _lastHourAngle) {
     drawHandDiff(
        HOUR_HAND_LENGTH,
        HOUR_HAND_WIDTH,
        hourAngle,
        _lastHourPixels,
        _lastHourPixelCount,
        HOUR_PIXEL_BUF_SIZE,
        _theme.handHour,
        _theme.background,
        isClippedBauhaus
      );
      drawCounterweight(hourAngle);
      _lastHourAngle = hourAngle;
    }

    if (minuteAngle != _lastMinuteAngle) {
      drawHandDiff(
        MINUTE_HAND_LENGTH,
        MINUTE_HAND_WIDTH,
        minuteAngle,
        _lastMinutePixels,
        _lastMinutePixelCount,
        MINUTE_PIXEL_BUF_SIZE,
        _theme.handMinute,
        _theme.background,
        isClippedBauhaus
      );
      _lastMinuteAngle = minuteAngle;
    }

    char timeBuf[6];
    sprintf(timeBuf, "%02d:%02d", timeinfo.tm_hour, timeinfo.tm_min);
    if (strcmp(timeBuf, _lastTimeText) != 0) {
      drawDigitalTime(timeinfo.tm_hour, timeinfo.tm_min);
      strncpy(_lastTimeText, timeBuf, sizeof(_lastTimeText) - 1);
    }
  }

  drawStatusDot(state, blinkState);
}

void ClockFaceBauhaus::drawBackground() {
  TFT_display.fillScreen(_theme.background);
}

void ClockFaceBauhaus::drawFaceRing() {
  for (int i = 0; i < 12; i++) {
    if (i == 6 || i == 3) {
      continue; // status dot occupies this position
    }
    float angle = (i * 30 - 90) * PI / 180.0f;
    int mx = CENTER_X + (int)roundf(MARKER_RADIUS * cosf(angle));
    int my = CENTER_Y + (int)roundf(MARKER_RADIUS * sinf(angle));
    bool isMajor = (i % 3 == 0);
    TFT_display.fillCircle(
      mx, my,
      isMajor ? MARKER_MAJOR_SIZE : MARKER_MINOR_SIZE,
      isMajor ? _theme.markerMajor : _theme.markerMinor
    );
    yield();
  }
}

void ClockFaceBauhaus::drawCounterweight(float hourAngleDeg) {
  float rad = (hourAngleDeg + 180.0f) * PI / 180.0f;
  int cx = CENTER_X + (int)roundf(COUNTERWEIGHT_DIST * cosf(rad));
  int cy = CENTER_Y + (int)roundf(COUNTERWEIGHT_DIST * sinf(rad));

  if (_lastCounterweightValid) {
    TFT_display.fillCircle(_lastCounterweightX, _lastCounterweightY, COUNTERWEIGHT_RADIUS, _theme.background);
    TFT_display.drawCircle(_lastCounterweightX, _lastCounterweightY, COUNTERWEIGHT_RADIUS, _theme.face);
    _lastCounterweightValid = false;
  }

  int dx = cx - CENTER_X;
  int dy = cy - CENTER_Y;
  if (dx * dx + dy * dy >= COUNTERWEIGHT_CLIP_R * COUNTERWEIGHT_CLIP_R) {
    return;
  }

  TFT_display.fillCircle(cx, cy, COUNTERWEIGHT_RADIUS, _theme.counterweight);
  TFT_display.drawCircle(cx, cy, COUNTERWEIGHT_RADIUS, _theme.face);
  _lastCounterweightX = (int16_t)cx;
  _lastCounterweightY = (int16_t)cy;
  _lastCounterweightValid = true;
}

void ClockFaceBauhaus::drawDigitalTime(int hour, int minute) {
  char buf[6];
  sprintf(buf, "%02d:%02d", hour, minute);
  TFT_display.fillRect(TIME_TEXT_X, TIME_TEXT_Y, TIME_TEXT_W, TIME_CHAR_H, _theme.background);
  TFT_display.setTextSize(3);
  TFT_display.setTextColor(_theme.face, _theme.background);
  TFT_display.setCursor(TIME_TEXT_X, TIME_TEXT_Y);
  TFT_display.print(buf);
}

void ClockFaceBauhaus::drawStatusDot(AppState state, bool blinkState) {
  uint16_t color;
  if (state == DISCONNECTED || state == NOT_CONFIGURED) {
    color = _theme.statusNoWifi;
  }
  else if (state == CONNECTED_SYNCING || isNtpSyncRequested()) {
    color = blinkState ? _theme.statusSyncing : _theme.background;
  }
  else {
    color = _theme.statusOk;
  }

  if (color != _lastStatusColor) {
    TFT_display.fillCircle(STATUS_DOT_X, STATUS_DOT_Y, STATUS_DOT_RADIUS, color);
    _lastStatusColor = color;
  }
}
