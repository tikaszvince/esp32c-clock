#include "clock_face_factory.h"
#include "clock_face_classic.h"

static ClockFaceClassic classicFace;

ClockFace* getInstance(ClockFaceType type) {
  return &classicFace;
}
