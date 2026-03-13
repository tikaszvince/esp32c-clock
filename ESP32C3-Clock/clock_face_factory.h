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
ClockFace* getFaceAt(int index);
ClockFaceType getTypeAt(int index);
int getFaceCount();
ClockFace* getFaceById(const char* id);
int getIndexById(const char* id);

#endif
