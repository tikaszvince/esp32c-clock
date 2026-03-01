#include <Arduino.h>
#include <time.h>
#include "clock_face_bauhaus_auto.h"

ClockFaceBauhausAuto::ClockFaceBauhausAuto()
  : _light(ClockFaceBauhaus::createLight()),
    _dark(ClockFaceBauhaus::createDark()),
    _active(nullptr) {
}

ClockFace* ClockFaceBauhausAuto::selectFace(int hour) {
  return (hour >= BAUHAUS_LIGHT_HOUR_START && hour < BAUHAUS_DARK_HOUR_START)
    ? &_light
    : &_dark;
}

void ClockFaceBauhausAuto::draw(AppState state, bool blinkState) {
  struct tm timeinfo;
  ClockFace* next;
  if (getLocalTime(&timeinfo)) {
    next = selectFace(timeinfo.tm_hour);
  }
  else {
    next = &_dark;
  }

  if (next != _active) {
    _active = next;
    _active->reset();
  }

  _active->draw(state, blinkState);
}

void ClockFaceBauhausAuto::reset() {
  if (_active != nullptr) {
    _active->reset();
  }
  else {
    _light.reset();
    _dark.reset();
  }
}
