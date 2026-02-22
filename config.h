#ifndef CONFIG_H
#define CONFIG_H

#include <WiFiManager.h>
#include <Preferences.h>

// Exposed config getters
String getTimezone();
String getNTPServer();

// Lifecycle
bool initConfig();
void resetConfig();

#endif
