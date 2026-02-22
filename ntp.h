#ifndef NTP_H
#define NTP_H

#include "states.h"

extern unsigned long lastNtp;
extern const unsigned long NTP_SYNC_INTERVAL;

NtpState getNtpState();

void syncTimeWithNTP(void (*onStatus)(const char*));

#endif
