#ifndef CLOCK_FACE_HELPERS_H
#define CLOCK_FACE_HELPERS_H

#include <cstdint>
#include <cmath>
#include "app_state.h"

struct Pixel {
  int16_t x;
  int16_t y;
};

inline float roundAngle(float x) {
  return std::floor((x * 10) + 0.5f) / 10;
}

int collectHandPixels(
  float angleDeg,
  int length,
  int width,
  Pixel* buf,
  int bufSize,
  bool (*clipFn)(int x, int y)
);

void drawHandDiff(
  int length,
  int width,
  float newAngle,
  Pixel* lastPixels,
  int& lastCount,
  int bufSize,
  uint16_t color,
  uint16_t backgroundColor,
  bool (*clipFn)(int x, int y)
);

void drawStatusIcons(
  AppState state,
  bool blinkState,
  int wifiX,
  int wifiY,
  int ntpX,
  int ntpY
);

#endif
