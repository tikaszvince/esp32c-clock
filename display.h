#ifndef DISPLAY_H
#define DISPLAY_H

#include <DIYables_TFT_Round.h>
#include "display_constants.h"

// Colors
#define COLOR_BACKGROUND  DIYables_TFT::colorRGB(0, 0, 0)       // Black
#define COLOR_CLOCKFACE   DIYables_TFT::colorRGB(255, 255, 255) // White
#define COLOR_HOUR_HAND   DIYables_TFT::colorRGB(255, 255, 255) // White
#define COLOR_MINUTE_HAND DIYables_TFT::colorRGB(80, 255, 255)  // Cyan
#define COLOR_SECOND_HAND DIYables_TFT::colorRGB(255, 0, 0)     // Red
#define COLOR_CENTER_DOT  DIYables_TFT::colorRGB(255, 0, 0)     // Red
#define COLOR_YELLOW      DIYables_TFT::colorRGB(255, 255, 0)   // Yellow
#define COLOR_RED         DIYables_TFT::colorRGB(255, 0, 0)

enum IconStatus { hide, flash, show };

extern int  iconStatusWifi;
extern bool iconStateWifi;
extern int  iconStatusSync;
extern bool iconStateSync;

extern DIYables_TFT_GC9A01_Round TFT_display;

void drawClockFace();
void drawTextBox();
void redrawTextBox(const char str[]);
void writeText(const char str[], bool center);
void displayDigitalTime(struct tm &timeinfo);
void updateClockDisplay();
void displayResetQuestion();
void displayWifiSetupInstructions();
void updateIcons();
void updateWifiIcon();
void updateSyncIcon();
void drawIcon(bool visible, int x, const uint16_t* icon);

#endif
