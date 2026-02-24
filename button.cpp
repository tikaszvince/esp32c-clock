#include "Arduino.h"
#include <OneButton.h>
#include "button.h"
#include "pins.h"
#include "app_state.h"  

#define LONG_PRESS_TIME  5000    // 5 seconds
#define RESET_TIMEOUT_MS 30000UL // 30 seconds

static OneButton buttonBoot(BOOT_BUTTON_PIN, true);
static unsigned long resetPendingStart = 0;

static void (*_onDoubleClick)() = nullptr;

static void bootButtonDoubleClick() {
  Serial.println("Boot button double click.");

  if (getAppState() == RESET_PENDING) {
    Serial.println("Reset confirmed! Calling resetConfig...");
    if (_onDoubleClick) {
      _onDoubleClick();
    }
  }
}

static void bootButtonLongPressStop() {
  Serial.println("Boot button longPress stop");
  setAppState(RESET_PENDING);
  resetPendingStart = millis();
  Serial.println("Reset pending — waiting for confirmation...");
}

void buttonSetup(
  void (*onDoubleClick)()
) {
  _onDoubleClick = onDoubleClick;
  buttonBoot.setLongPressIntervalMs(LONG_PRESS_TIME);
  buttonBoot.attachDoubleClick(bootButtonDoubleClick);
  buttonBoot.attachLongPressStop(bootButtonLongPressStop);
}

void buttonLoop() {
  buttonBoot.tick();

  // Check for reset confirmation timeout
  if (getAppState() == RESET_PENDING) {
    if (millis() - resetPendingStart >= RESET_TIMEOUT_MS) {
      Serial.println("Reset confirmation timed out. Returning to previous state.");
      setAppState(getPreviousState());
    }
  }
}
