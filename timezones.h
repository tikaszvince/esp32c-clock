#ifndef TIMEZONES_H
#define TIMEZONES_H

// Buffer size for the generated <select> HTML.
// Increase if more timezone entries are added.
static const int TIMEZONE_SELECT_BUFFER_SIZE = 7 * 1024; // 7 KB

void buildTimezoneSelect(
  const char* name,
  const char* currentPosix,
  char* buf,
  int bufSize
);

#endif
