#ifndef CLOCK_FACE_FACTORY_H
#define CLOCK_FACE_FACTORY_H

#include "clock_face.h"

enum ClockFaceType {
  CLOCK_FACE_CLASSIC,
  CLOCK_FACE_ORBIT,
  CLOCK_FACE_BAUHAUS_LIGHT,
  CLOCK_FACE_BAUHAUS_DARK,
  CLOCK_FACE_BAUHAUS_AUTO,
};

ClockFace* getInstance(ClockFaceType type);

#endif
