#include "Arduino.h"
#include "button.h"
#include "config.h"
#include <OneButton.h>

#define BOOT_BUTTON_PIN 0 // GPIO0 pin, the BOOT button.
#define SHORT_PRESS_TIME 500 // 500 milliseconds
#define LONG_PRESS_TIME  5000 // 5 seconds

static OneButton buttonBoot(BOOT_BUTTON_PIN, true);

static void bootButtonClick() {
  Serial.println("Boot button click.");
}

static void bootButtonDoubleClick() {
  Serial.println("Boot button double click.");
}

static void bootButtonLongPressStart() {
  Serial.println("Boot button longPress start");
}

static void bootButtonLongPressStop() {
  Serial.println("Boot button longPress stop");
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
}