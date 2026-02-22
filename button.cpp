#include "Arduino.h"
#include <OneButton.h>
#include "button.h"
#include "pins.h"

#define SHORT_PRESS_TIME 400     // 400 milliseconds
#define LONG_PRESS_TIME  5000    // 5 seconds
#define RESET_TIMEOUT_MS 30000UL // 30 seconds

static OneButton buttonBoot(BOOT_BUTTON_PIN, true);
static ButtonMode currentMode = NORMAL;
static unsigned long resetPendingStart = 0;

static void (*_onDoubleClick)()  = nullptr;
static void (*_onLongPressStop)() = nullptr;

ButtonMode getButtonMode() {
  return currentMode;
}

static void bootButtonDoubleClick() {
  Serial.println("Boot button double click.");

  if (currentMode == RESET_PENDING) {
    Serial.println("Reset confirmed! Calling resetConfig...");
    if (_onDoubleClick) {
      _onDoubleClick();
    }
  }
}

static void bootButtonLongPressStop() {
  Serial.println("Boot button longPress stop");
  currentMode = RESET_PENDING;
  resetPendingStart = millis();
  Serial.println("Reset pending — waiting for confirmation...");
  if (_onLongPressStop) {
    _onLongPressStop();
  }
}

void buttonSetup(
  void (*onDoubleClick)(),
  void (*onLongPressStop)()
) {
  _onDoubleClick  = onDoubleClick;
  _onLongPressStop = onLongPressStop;

  //buttonBoot.setLongPressIntervalMs(LONG_PRESS_TIME);
  buttonBoot.attachDoubleClick(bootButtonDoubleClick);
  buttonBoot.attachLongPressStop(bootButtonLongPressStop);
}

void buttonLoop() {
  buttonBoot.tick();

  // Check for reset confirmation timeout
  if (currentMode == RESET_PENDING) {
    if (millis() - resetPendingStart >= RESET_TIMEOUT_MS) {
      Serial.println("Reset confirmation timed out. Returning to normal.");
      currentMode = NORMAL;
    }
  }
}
