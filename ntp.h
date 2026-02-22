#ifndef NTP_H
#define NTP_H

enum NtpStatus { NTP_IDLE, NTP_SYNCING };

extern unsigned long lastNtp;
extern const unsigned long NTP_SYNC_INTERVAL;

NtpStatus getNtpStatus();

void syncTimeWithNTP(void (*onStatus)(const char*));

#endif
