#ifndef NTP_H
#define NTP_H

void syncTimeWithNTP(void (*onStatus)(const char*));
void ntpTaskStart();

#endif
