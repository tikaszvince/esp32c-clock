#ifndef CLOCK_FACE_H
#define CLOCK_FACE_H

#include <time.h>
#include "app_state.h"

struct DrawContext {
  AppState state;
  bool blinkState;
  tm timeinfo;
  bool gracePeriodActive;
};

class ClockFace {
public:
  virtual void draw(const DrawContext& ctx) = 0;
  virtual void reset() = 0;

  virtual const char* getId() const = 0;
  virtual const char* getName() const = 0;

  #if ENCODER_ENABLED
    virtual bool handlesGracePeriodOverlay() const = 0;
  #endif

  virtual ~ClockFace() {}
};

#endif
