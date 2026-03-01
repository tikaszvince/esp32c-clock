#include "Arduino.h"
#include <OneButton.h>
#include "button.h"
#include "pins.h"
#include "app_state.h"
#include "timing_constants.h"

static OneButton buttonBoot(BOOT_BUTTON_PIN, true);
static unsigned long resetPendingStart = 0;

static void (*_onResetConfirm)() = nullptr;
static void (*_onDoubleClick)() = nullptr;

static void bootButtonDoubleClick() {
  Serial.println("Boot button double click.");

  if (getAppState() == RESET_PENDING) {
    Serial.println("Reset confirmed! Calling resetConfig...");
    if (_onResetConfirm) {
      _onResetConfirm();
    }
  }
  else {
    Serial.println("Double click in normal state.");
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
  void (*onResetConfirm)(),
  void (*onDoubleClick)()
) {
  _onResetConfirm = onResetConfirm;
  _onDoubleClick = onDoubleClick;
  buttonBoot.setDebounceMs(BUTTON_DEBOUNCE_MS);
  buttonBoot.setClickMs(BUTTON_CLICK_MS);
  buttonBoot.setPressMs(BUTTON_PRESS_TICK_MS);
  buttonBoot.setLongPressIntervalMs(LONG_PRESS_TIME_MS);
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
