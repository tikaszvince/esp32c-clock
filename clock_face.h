#ifndef CLOCK_FACE_H
#define CLOCK_FACE_H

#include "app_state.h"

class ClockFace {
public:
  virtual void draw(AppState state, bool blinkState) = 0;
  virtual void reset() = 0;
  virtual ~ClockFace() {}
};

#endif
