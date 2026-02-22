#include "Arduino.h"
#include "button.h"
#include "config.h"
#include "pins.h"
#include <OneButton.h>

#define SHORT_PRESS_TIME 500     // 500 milliseconds
#define LONG_PRESS_TIME  5000    // 5 seconds
#define RESET_TIMEOUT_MS 30000UL // 30 seconds

static OneButton buttonBoot(BOOT_BUTTON_PIN, true);
static ButtonMode currentMode = NORMAL;
static unsigned long resetPendingStart = 0;

ButtonMode getButtonMode() {
  return currentMode;
}

static void bootButtonClick() {
  Serial.println("Boot button click.");
}

static void bootButtonDoubleClick() {
  Serial.println("Boot button double click.");

  if (currentMode == RESET_PENDING) {
    Serial.println("Reset confirmed! Calling resetConfig...");
    resetConfig();
  }
}

static void bootButtonLongPressStart() {
  Serial.println("Boot button longPress start");
}

static void bootButtonLongPressStop() {
  Serial.println("Boot button longPress stop");
  currentMode = RESET_PENDING;
  resetPendingStart = millis();
  Serial.println("Reset pending — waiting for confirmation...");
}

void buttonSetup() {
  buttonBoot.setLongPressIntervalMs(LONG_PRESS_TIME);
  buttonBoot.attachClick(bootButtonClick);
  buttonBoot.attachDoubleClick(bootButtonDoubleClick);
  buttonBoot.attachLongPressStart(bootButtonLongPressStart);
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
