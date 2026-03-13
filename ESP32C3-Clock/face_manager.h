#ifndef FACE_MANAGER_H
#define FACE_MANAGER_H

#include "clock_face_factory.h"

void setConfiguredClockFace();

#if ENCODER_ENABLED
  void faceManagerSetup(ClockFaceType defaultType);
  void faceManagerOnRotation(int delta);
  void faceManagerOnSingleClick();
  void faceManagerUpdate();
  bool faceManagerIsGracePeriodActive();
  float faceManagerGetGracePeriodFraction();
#endif

#endif
