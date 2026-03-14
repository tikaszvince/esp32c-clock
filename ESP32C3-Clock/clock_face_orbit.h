#ifndef CLOCK_FACE_ORBIT_H
#define CLOCK_FACE_ORBIT_H

#include <time.h>
#include "clock_face.h"
#include "app_state.h"

class ClockFaceOrbit : public ClockFace {
public:
  void draw(const DrawContext& ctx) override;
  void reset() override;

  const char* getId() const override;
  const char* getName() const override;
  #if !DISABLE_ENCODER
    bool handlesGracePeriodOverlay() const override;
  #endif

private:
  bool _needsFullRedraw = true;
  int _lastMinute = -2;
  int _lastDay = -2;
  int _lastValidHour = -1;
  int _lastValidMinute = -1;
  AppState _lastState = NOT_CONFIGURED;
  bool _lastBlinkState = false;

  void drawBackground();
  void drawArcTrack(const struct tm* timeinfo, int displayMinute);
  void drawTime(int hour, int minute);
  void drawDate(const struct tm* timeinfo);
  void drawIcons(AppState state, bool blinkState);
  void drawDaySegments(int dayOfWeek);
};

#endif
