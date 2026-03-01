#include "Arduino.h"
#include "app_state.h"
#include "timing_constants.h"

#define STATUS_TEXT_MAX_LENGTH 32

static bool inited = false;

static AppState currentState = NOT_CONFIGURED;
static AppState previousState = NOT_CONFIGURED;

static unsigned long lastNtpSync = 0;
static unsigned long lastReconnectAttempt = 0;

static char statusText[STATUS_TEXT_MAX_LENGTH] = "";
static unsigned long statusTextExpiry = 0;

static bool ntpSyncRequested = false;

void setInited() {
  inited = true;
  Serial.println("App inited.");
}

bool isInited() {
  return inited;
}

void setAppState(AppState newState) {
  if (newState != currentState) {
    previousState = currentState;
    currentState  = newState;
    Serial.print("AppState changed: ");
    Serial.println(currentState);
  }
}

AppState getAppState() {
  return currentState;
}

AppState getPreviousState() {
  return previousState;
}

void updateLastNtpSync() {
  lastNtpSync = millis();
  Serial.println("NTP sync timestamp updated.");
}

void updateLastReconnectAttempt() {
  lastReconnectAttempt = millis();
  Serial.println("Reconnect attempt timestamp updated.");
}

bool isNtpSyncDue() {
  if (lastNtpSync == 0) {
    return true;
  }
  return (millis() - lastNtpSync) >= NTP_SYNC_INTERVAL_MS;
}

bool isReconnectDue() {
  if (lastReconnectAttempt == 0) {
    return true;
  }
  return (millis() - lastReconnectAttempt) >= RECONNECT_INTERVAL_MS;
}

void setStatusText(const char* text, unsigned long timeoutMs) {
  strncpy(statusText, text, STATUS_TEXT_MAX_LENGTH - 1);
  statusText[STATUS_TEXT_MAX_LENGTH - 1] = '\0';
  statusTextExpiry = millis() + timeoutMs;
  Serial.print("StatusText set: ");
  Serial.println(statusText);
}

const char* getStatusText() {
  return statusText;
}

bool isStatusTextActive() {
  if (strlen(statusText) == 0) {
    return false;
  }
  if (millis() >= statusTextExpiry) {
    statusText[0] = '\0';
    return false;
  }
  return true;
}

void requestNtpSync() {
  ntpSyncRequested = true;
  Serial.println("NTP sync requested.");
}

bool isNtpSyncRequested() {
  return ntpSyncRequested;
}

void clearNtpSyncRequest() {
  ntpSyncRequested = false;
}
