#ifndef APP_STATE_H
#define APP_STATE_H

enum AppState {
  NOT_CONFIGURED,
  CONNECTING,
  CONNECTED_NOT_SYNCED,
  CONNECTED_SYNCING,
  CONNECTED_SYNCED,
  DISCONNECTED,
  RESET_PENDING
};

void setAppState(AppState newState);
AppState getAppState();
AppState getPreviousState();

void setInited();
bool isInited();

void updateLastNtpSync();
void updateLastReconnectAttempt();
bool isNtpSyncDue();
bool isReconnectDue();

void setStatusText(const char* text, unsigned long timeoutMs);
const char* getStatusText();
bool isStatusTextActive();

#endif
