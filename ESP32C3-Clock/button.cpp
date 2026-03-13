#include "Arduino.h"
#include <OneButton.h>
#include "button.h"
#include "pins.h"
#include "app_state.h"
#include "timing_constants.h"

static OneButton buttonBoot(BOOT_BUTTON_PIN, true);
#if ENCODER_ENABLED
  static OneButton buttonEncoder(PIN_ENCODER_SW, true);
#endif

static unsigned long resetPendingStart = 0;

static void (*_onResetConfirm)() = nullptr;
static void (*_onDoubleClick)() = nullptr;
static void (*_onSingleClick)() = nullptr;
#if ENCODER_ENABLED
  static void (*_onRotation)(int delta) = nullptr;
#endif

static void handleDoubleClick() {
  Serial.println("Button double click.");
  if (getAppState() == RESET_PENDING) {
    Serial.println("Reset confirmed!");
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

static void handleLongPressStop() {
  Serial.println("Button long press stop.");
  setAppState(RESET_PENDING);
  resetPendingStart = millis();
  Serial.println("Reset pending — waiting for confirmation...");
}

#if ENCODER_ENABLED
  static void handleSingleClick() {
    Serial.println("Encoder button single click.");
    if (_onSingleClick) {
      _onSingleClick();
    }
  }
#endif

void buttonSetup(
  void (*onResetConfirm)(),
  void (*onDoubleClick)(),
  void (*onSingleClick)(),
  void (*onRotation)(int delta)
) {
  _onResetConfirm = onResetConfirm;
  _onDoubleClick = onDoubleClick;
  _onSingleClick = onSingleClick;
  #if ENCODER_ENABLED
    pinMode(PIN_ENCODER_CLK, INPUT);
    pinMode(PIN_ENCODER_DT, INPUT);
    _onRotation = onRotation;
  #endif

  buttonBoot.setDebounceMs(BUTTON_DEBOUNCE_MS);
  buttonBoot.setClickMs(BUTTON_CLICK_MS);
  buttonBoot.setPressMs(BUTTON_PRESS_TICK_MS);
  buttonBoot.setLongPressIntervalMs(LONG_PRESS_TIME_MS);
  buttonBoot.attachDoubleClick(handleDoubleClick);
  buttonBoot.attachLongPressStop(handleLongPressStop);

  #if ENCODER_ENABLED
    buttonEncoder.setDebounceMs(BUTTON_DEBOUNCE_MS);
    buttonEncoder.setClickMs(BUTTON_CLICK_MS);
    buttonEncoder.setPressMs(BUTTON_PRESS_TICK_MS);
    buttonEncoder.setLongPressIntervalMs(LONG_PRESS_TIME_MS);
    buttonEncoder.attachClick(handleSingleClick);
    buttonEncoder.attachDoubleClick(handleDoubleClick);
    buttonEncoder.attachLongPressStop(handleLongPressStop);
  #endif
}

void buttonLoop() {
  buttonBoot.tick();

  #if ENCODER_ENABLED
    buttonEncoder.tick();

    // Rotation polling.
    static int lastClk = -1;
    static unsigned long lastRotationMs = 0;

    int clk = digitalRead(PIN_ENCODER_CLK);
    if (lastClk == -1) {
      lastClk = clk;
    }
    if (clk != lastClk) {
      lastClk = clk;
      if (clk == LOW) {
        unsigned long now = millis();
        if (now - lastRotationMs >= ENCODER_DEBOUNCE_MS) {
          lastRotationMs = now;
          int dt = digitalRead(PIN_ENCODER_DT);
          int delta = (dt == HIGH) ? 1 : -1;
          Serial.print("Rotation "); Serial.println(delta);
          if (_onRotation) {
            _onRotation(delta);
          }
        }
      }
    }
  #endif

  // Reset confirmation timeout — shared by both buttons.
  if (getAppState() == RESET_PENDING) {
    if (millis() - resetPendingStart >= RESET_TIMEOUT_MS) {
      Serial.println("Reset confirmation timed out. Returning to previous state.");
      setAppState(getPreviousState());
    }
  }
}
