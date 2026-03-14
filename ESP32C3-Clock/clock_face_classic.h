#ifndef CLOCK_FACE_CLASSIC_H
#define CLOCK_FACE_CLASSIC_H

#include <cstdint>
#include <time.h>
#include "clock_face.h"
#include "clock_face_helpers.h"
#include "app_state.h"

class ClockFaceClassic : public ClockFace {
public:
  void draw(const DrawContext& ctx) override;
  void reset() override;

private:
  bool _needsFullRedraw = true;
  char _lastText[16] = "";
  float _lastHourAngle = -1.0f;
  float _lastMinuteAngle = -1.0f;

  static const int HOUR_PIXEL_BUF_SIZE = 300;
  static const int MINUTE_PIXEL_BUF_SIZE = 320;

  Pixel _lastHourPixels[HOUR_PIXEL_BUF_SIZE];
  int _lastHourPixelCount = 0;
  Pixel _lastMinutePixels[MINUTE_PIXEL_BUF_SIZE];
  int _lastMinutePixelCount = 0;

  void drawBackground();
  void drawClockFace();
  void drawTextBoxFrame();
  void drawTextBoxContent(AppState state, tm timeinfo);
  void drawIcons(AppState state, bool blinkState);
  void drawHands(tm timeinfo);

  const char* getId() const override;
  const char* getName() const override;
  #if !DISABLE_ENCODER
    bool handlesGracePeriodOverlay() const override;
  #endif
};

#endif
