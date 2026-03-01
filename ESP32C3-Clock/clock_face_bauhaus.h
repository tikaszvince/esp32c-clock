#ifndef CLOCK_FACE_BAUHAUS_H
#define CLOCK_FACE_BAUHAUS_H

#include <cstdint>
#include <time.h>
#include "clock_face.h"
#include "clock_face_helpers.h"
#include "app_state.h"

struct BauhausTheme {
  uint16_t background;
  uint16_t face;
  uint16_t handHour;
  uint16_t handMinute;
  uint16_t counterweight;
  uint16_t markerMajor;
  uint16_t markerMinor;
  uint16_t statusOk;
  uint16_t statusNoWifi;
  uint16_t statusSyncing;
};

class ClockFaceBauhaus : public ClockFace {
public:
  static ClockFaceBauhaus createLight();
  static ClockFaceBauhaus createDark();

  void draw(AppState state, bool blinkState) override;
  void reset() override;

private:
  explicit ClockFaceBauhaus(const BauhausTheme& theme);

  BauhausTheme _theme;

  bool _needsFullRedraw = true;
  float _lastHourAngle = -1.0f;
  float _lastMinuteAngle = -1.0f;
  uint16_t _lastStatusColor = 0;
  char _lastTimeText[6] = "";

  bool _lastCounterweightValid = false;
  int16_t _lastCounterweightX = 0;
  int16_t _lastCounterweightY = 0;

  static const int HOUR_PIXEL_BUF_SIZE   = 350;
  static const int MINUTE_PIXEL_BUF_SIZE = 380;

  Pixel _lastHourPixels[HOUR_PIXEL_BUF_SIZE];
  int _lastHourPixelCount = 0;
  Pixel _lastMinutePixels[MINUTE_PIXEL_BUF_SIZE];
  int _lastMinutePixelCount = 0;

  void drawBackground();
  void drawFaceRing();
  void drawHands();
  void drawCounterweight(float hourAngleDeg);
  void drawDigitalTime(int hour, int minute);
  void drawStatusDot(AppState state, bool blinkState);
};

#endif
