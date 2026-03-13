#include <Arduino.h>
#include <time.h>
#include "clock_face_bauhaus_auto.h"

static constexpr int BAUHAUS_LIGHT_HOUR_START = 7;
static constexpr int BAUHAUS_DARK_HOUR_START  = 19;

const char* ClockFaceBauhausAuto::getId() const {
  return "bauhaus_auto";
}

const char* ClockFaceBauhausAuto::getName() const {
  return "Bauhaus (auto light/dark)";
}
#if ENCODER_ENABLED
  bool ClockFaceBauhausAuto::handlesGracePeriodOverlay() const {
    return false;
  }
#endif

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

void ClockFaceBauhausAuto::draw(
  AppState state,
  bool blinkState,
  tm timeinfo
) {
  ClockFace* next;
  next = selectFace(timeinfo.tm_hour);

  if (next != _active) {
    _active = next;
    _active->reset();
  }

  _active->draw(state, blinkState, timeinfo);
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
