#include "Arduino.h"
#include "face_manager.h"
#include "clock_face_factory.h"
#include "display.h"
#include "app_state.h"
#include "timing_constants.h"
#include "config.h"
#include "pins.h"

void setConfiguredClockFace() {
  String id = getDefaultFaceId();
  int index = getIndexById(id.c_str());
  if (index < 0) {
    index = getIndexById("orbit");
  }
  if (index < 0) {
    index = 0;
  }

  #if ENCODER_ENABLED
    faceManagerSetup(getTypeAt(index));
  #else
    ClockFace* face = getFaceAt(index);
    face->reset();
    setClockFace(face);
  #endif
}

#if ENCODER_ENABLED

  static int _defaultIndex = 0;
  static int _currentIndex = 0;
  static unsigned long _gracePeriodStart = 0;

  static int findIndexByType(ClockFaceType type) {
    for (int i = 0; i < getFaceCount(); i++) {
      if (getTypeAt(i) == type) {
        return i;
      }
    }
    return 0;
  }

  void faceManagerSetup(ClockFaceType defaultType) {
    _defaultIndex = findIndexByType(defaultType);
    _currentIndex = _defaultIndex;
    _gracePeriodStart = 0;
    setClockFace(getFaceAt(_currentIndex));
  }

  void faceManagerOnRotation(int delta) {
    AppState state = getAppState();
    Serial.print("Face manager rotation. Current state: "); Serial.println(state);
    if (
      state == RESET_PENDING
      || state == NOT_CONFIGURED
      || state == CONNECTING
    ) {
      Serial.println("Ignore because state.");
      return;
    }

    int count = getFaceCount();
    _currentIndex = (_currentIndex + delta + count) % count;

    ClockFace* face = getFaceAt(_currentIndex);
    face->reset();
    Serial.print("Set face: "); Serial.println(face->getId());
    setClockFace(face);
    _gracePeriodStart = millis();
  }

  void faceManagerOnSingleClick() {
    if (_gracePeriodStart == 0) {
      return;
    }
    static unsigned long lastSaveMs = 0;
    if (millis() - lastSaveMs < 500) {
      return;
    }
    lastSaveMs = millis();
    const char* id = getFaceAt(_currentIndex)->getId();
    saveDefaultFaceId(id);
    _defaultIndex = _currentIndex;
    _gracePeriodStart = 0;
    getFaceAt(_currentIndex)->reset();
    Serial.print("Default face saved: ");
    Serial.println(id);
  }

  void faceManagerUpdate() {
    if (_gracePeriodStart == 0) {
      return;
    }

    if ((millis() - _gracePeriodStart) >= FACE_GRACE_PERIOD_MS) {
      Serial.println("Grace period expired. Reverting face.");
      _gracePeriodStart = 0;
      _currentIndex = _defaultIndex;
      ClockFace* face = getFaceAt(_currentIndex);
      face->reset();
      setClockFace(face);
    }
  }

  bool faceManagerIsGracePeriodActive() {
    return _gracePeriodStart != 0;
  }

  float faceManagerGetGracePeriodFraction() {
    if (_gracePeriodStart == 0) {
      return 0.0f;
    }
    unsigned long elapsed = millis() - _gracePeriodStart;
    if (elapsed >= FACE_GRACE_PERIOD_MS) {
      return 0.0f;
    }
    return 1.0f - (float)elapsed / (float)FACE_GRACE_PERIOD_MS;
  }

#endif
