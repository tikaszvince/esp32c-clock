#include "Arduino.h"
#include "display_task.h"
#include "display.h"
#include "timing_constants.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"

static TaskHandle_t displayTaskHandle = NULL;

static void displayTask(void* parameter) {
  for (;;) {
    takeDisplayMutex();
    redrawDisplay();
    giveDisplayMutex();
    vTaskDelay(pdMS_TO_TICKS(BLINK_INTERVAL_MS));
  }
}

void displayTaskStart() {
  xTaskCreatePinnedToCore(
    displayTask,
    "DisplayTask",
    2048,
    NULL,
    1,
    &displayTaskHandle,
    1  // core 1
  );
  Serial.println("Display task started on core 1.");
}

void displayTaskStop() {
  if (displayTaskHandle != NULL) {
    vTaskDelete(displayTaskHandle);
    displayTaskHandle = NULL;
    Serial.println("Display task stopped.");
  }
}
