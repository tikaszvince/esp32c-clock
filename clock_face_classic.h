#ifndef CLOCK_FACE_CLASSIC_H
#define CLOCK_FACE_CLASSIC_H

#include "clock_face.h"

class ClockFaceClassic : public ClockFace {
public:
  void draw(bool blinkState) override;
};

#endif
