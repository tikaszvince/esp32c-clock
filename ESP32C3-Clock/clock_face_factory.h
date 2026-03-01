#ifndef CLOCK_FACE_FACTORY_H
#define CLOCK_FACE_FACTORY_H

#include "clock_face.h"

enum ClockFaceType {
  CLOCK_FACE_CLASSIC,
};

ClockFace* getInstance(ClockFaceType type);

#endif
