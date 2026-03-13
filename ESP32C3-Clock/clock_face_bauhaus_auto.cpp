#include <Arduino.h>
#include <time.h>
#include "clock_face_bauhaus_auto.h"

static constexpr int BAUHAUS_LIGHT_HOUR_START = 7;
static constexpr int BAUHAUS_DARK_HOUR_START = 19;
static constexpr unsigned long PREVIEW_SWITCH_INTERVAL_MS = 5000UL;

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
  const DrawContext& ctx
) {
  AppState state = ctx.state;
  bool blinkState = ctx.blinkState;
  tm timeinfo = ctx.timeinfo;
  ClockFace* next;

  #if ENCODER_ENABLED
    if (ctx.gracePeriodActive) {
      unsigned long now = millis();
      if (_previewSwitchMs == 0 || (now - _previewSwitchMs) >= PREVIEW_SWITCH_INTERVAL_MS) {
        _previewShowLight = (_previewSwitchMs == 0) ? true : !_previewShowLight;
        _previewSwitchMs = now;
      }
      next = _previewShowLight ? &_light : &_dark;
    }
    else {
      _previewSwitchMs = 0;
      next = selectFace(timeinfo.tm_hour);
    }
  #else
    next = selectFace(timeinfo.tm_hour);
  #endif

  if (next != _active) {
    _active = next;
    _active->reset();
  }

  _active->draw(ctx);
}

void ClockFaceBauhausAuto::reset() {
  #if ENCODER_ENABLED
    _previewSwitchMs = 0;
    _previewShowLight = true;
  #endif
  if (_active != nullptr) {
    _active->reset();
  }
  else {
    _light.reset();
    _dark.reset();
  }
}
