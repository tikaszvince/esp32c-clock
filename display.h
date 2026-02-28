#ifndef DISPLAY_H
#define DISPLAY_H

#include <DIYables_TFT_Round.h>
#include "display_constants.h"
#include "clock_face.h"

// Colors
#define COLOR_BACKGROUND DIYables_TFT::colorRGB(0, 0, 0)
#define COLOR_CLOCKFACE DIYables_TFT::colorRGB(255, 255, 255)
#define COLOR_MINUTE_HAND DIYables_TFT::colorRGB(0, 255, 136)
#define COLOR_YELLOW DIYables_TFT::colorRGB(255, 255, 0)
#define COLOR_RED DIYables_TFT::colorRGB(255, 0, 0)

extern DIYables_TFT_GC9A01_Round TFT_display;

void takeDisplayMutex();
void giveDisplayMutex();

void displaySetup();

void setClockFace(ClockFace* face);
void redrawDisplay();

void displayResetQuestion();
void displayWifiSetupInstructions();


#endif
