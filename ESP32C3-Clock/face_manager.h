#ifndef FACE_MANAGER_H
#define FACE_MANAGER_H

#include "clock_face_factory.h"

void setConfiguredClockFace();

#if !DISABLE_ENCODER
  void faceManagerSetup(ClockFaceType defaultType);
  void faceManagerOnRotation(int delta);
  void faceManagerOnSingleClick();
  void faceManagerUpdate();
  bool faceManagerIsGracePeriodActive();
  float faceManagerGetGracePeriodFraction();
#endif

#endif
