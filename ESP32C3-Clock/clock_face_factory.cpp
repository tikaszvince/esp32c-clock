#include "clock_face_factory.h"
#include "clock_face_classic.h"
#include "clock_face_orbit.h"

static ClockFaceClassic classicFace;
static ClockFaceOrbit orbitFace;

ClockFace* getInstance(ClockFaceType type) {
  switch (type) {
    case CLOCK_FACE_ORBIT:
      return &orbitFace;

    case CLOCK_FACE_CLASSIC:
    default:
      return &classicFace;
  }
}
