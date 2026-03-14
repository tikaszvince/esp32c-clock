#ifndef CLOCK_FACE_BAUHAUS_AUTO_H
#define CLOCK_FACE_BAUHAUS_AUTO_H

#include <time.h>
#include "clock_face.h"
#include "clock_face_bauhaus.h"
#include "app_state.h"

class ClockFaceBauhausAuto : public ClockFace {
public:
  ClockFaceBauhausAuto();
  void draw(const DrawContext& ctx) override;
  void reset() override;

  const char* getId() const override;
  const char* getName() const override;
  #if !DISABLE_ENCODER
    bool handlesGracePeriodOverlay() const override;
  #endif

private:
  ClockFaceBauhaus _light;
  ClockFaceBauhaus _dark;
  ClockFace* _active;

  ClockFace* selectFace(int hour);

  #if !DISABLE_ENCODER
    unsigned long _previewSwitchMs;
    bool _previewShowLight;
  #endif
};

#endif
