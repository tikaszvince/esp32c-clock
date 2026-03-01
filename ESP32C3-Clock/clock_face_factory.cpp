#include "clock_face_factory.h"
#include "clock_face_classic.h"
#include "clock_face_orbit.h"
#include "clock_face_bauhaus.h"
#include "clock_face_bauhaus_auto.h"

static ClockFaceClassic classicFace;
static ClockFaceOrbit orbitFace;
static ClockFaceBauhaus bauhausLight = ClockFaceBauhaus::createLight();
static ClockFaceBauhaus bauhausDark = ClockFaceBauhaus::createDark();
static ClockFaceBauhausAuto bauhausAuto;

ClockFace* getInstance(ClockFaceType type) {
  switch (type) {
    case CLOCK_FACE_ORBIT:
      return &orbitFace;

    case CLOCK_FACE_BAUHAUS_LIGHT:
      return &bauhausLight;

    case CLOCK_FACE_BAUHAUS_DARK:
      return &bauhausDark;

    case CLOCK_FACE_BAUHAUS_AUTO:
      return &bauhausAuto;

    case CLOCK_FACE_CLASSIC:
    default:
      return &classicFace;
  }
}
