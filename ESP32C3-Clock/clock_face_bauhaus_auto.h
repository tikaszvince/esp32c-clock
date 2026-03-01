#ifndef CLOCK_FACE_BAUHAUS_AUTO_H
#define CLOCK_FACE_BAUHAUS_AUTO_H

#include "clock_face.h"
#include "clock_face_bauhaus.h"
#include "app_state.h"

static const int BAUHAUS_LIGHT_HOUR_START = 7;
static const int BAUHAUS_DARK_HOUR_START  = 19;

class ClockFaceBauhausAuto : public ClockFace {
public:
  ClockFaceBauhausAuto();
  void draw(AppState state, bool blinkState) override;
  void reset() override;

private:
  ClockFaceBauhaus _light;
  ClockFaceBauhaus _dark;
  ClockFace* _active;

  ClockFace* selectFace(int hour);
};

#endif
