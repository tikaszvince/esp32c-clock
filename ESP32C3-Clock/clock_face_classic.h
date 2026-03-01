#ifndef CLOCK_FACE_CLASSIC_H
#define CLOCK_FACE_CLASSIC_H

#include <cstdint>
#include "clock_face.h"
#include "clock_face_helpers.h"
#include "app_state.h"

class ClockFaceClassic : public ClockFace {
public:
  void draw(AppState state, bool blinkState) override;
  void reset() override;

private:
  bool _needsFullRedraw = true;
  char _lastText[16] = "";
  float _lastHourAngle = 0;
  float _lastMinuteAngle = 0;

  static const int HOUR_PIXEL_BUF_SIZE = 300;
  static const int MINUTE_PIXEL_BUF_SIZE = 320;

  Pixel _lastHourPixels[HOUR_PIXEL_BUF_SIZE];
  int _lastHourPixelCount = 0;
  Pixel _lastMinutePixels[MINUTE_PIXEL_BUF_SIZE];
  int _lastMinutePixelCount = 0;

  void drawBackground();
  void drawClockFace();
  void drawTextBoxFrame();
  void drawTextBoxContent(AppState state);
  void drawIcons(AppState state, bool blinkState);
  void drawHands();
};

#endif
