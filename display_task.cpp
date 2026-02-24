#include "Arduino.h"
#include "display_task.h"
#include "display.h"
#include "display_constants.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"

static TaskHandle_t displayTaskHandle = NULL;

static void displayTask(void* parameter) {
  for (;;) {
    takeDisplayMutex();
    updateIcons();
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
    0
  );
  Serial.println("Display task started.");
}
