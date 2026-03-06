#ifndef DISPLAY_H
#define DISPLAY_H

#include <time.h>
#include <DIYables_TFT_Round.h>
#include "display_constants.h"
#include "clock_face.h"

#if SCREENSHOT_MODE

#define CAPTURE_STRIP_HEIGHT 16

class CapturableTFT : public DIYables_TFT_GC9A01_Round {
public:
  CapturableTFT(uint8_t resPin, uint8_t dcPin, uint8_t csPin)
    : DIYables_TFT_GC9A01_Round(resPin, dcPin, csPin),
      _captureActive(false),
      _stripY0(0),
      _stripY1(0) {}

  void drawPixel(int16_t x, int16_t y, uint16_t color) override {
    DIYables_TFT_GC9A01_Round::drawPixel(x, y, color);
    if (_captureActive && y >= _stripY0 && y < _stripY1) {
      _stripBuffer[(y - _stripY0) * SCREEN_WIDTH + x] = color;
    }
  }

  void fillScreen(uint16_t color) override {
    DIYables_TFT_GC9A01_Round::fillScreen(color);
    if (_captureActive) {
      for (int i = 0; i < SCREEN_WIDTH * CAPTURE_STRIP_HEIGHT; i++) {
        _stripBuffer[i] = color;
      }
    }
  }

  void beginCapture(int stripY0) {
    _stripY0 = stripY0;
    _stripY1 = stripY0 + CAPTURE_STRIP_HEIGHT;
    memset(_stripBuffer, 0, sizeof(_stripBuffer));
    _captureActive = true;
  }

  void endCapture() {
    _captureActive = false;
  }

  const uint16_t* stripBuffer() const {
    return _stripBuffer;
  }

private:
  bool _captureActive;
  int _stripY0;
  int _stripY1;
  uint16_t _stripBuffer[SCREEN_WIDTH * CAPTURE_STRIP_HEIGHT];
};

#endif

// Colors
#define COLOR_BACKGROUND DIYables_TFT::colorRGB(0, 0, 0)
#define COLOR_CLOCKFACE DIYables_TFT::colorRGB(255, 255, 255)
#define COLOR_MINUTE_HAND DIYables_TFT::colorRGB(0, 255, 136)
#define COLOR_YELLOW DIYables_TFT::colorRGB(255, 255, 0)
#define COLOR_RED DIYables_TFT::colorRGB(255, 0, 0)

#if SCREENSHOT_MODE
  extern CapturableTFT TFT_display;
  void screenshotCaptureStrip(int stripY0, uint16_t* outBuffer);
#else
  extern DIYables_TFT_GC9A01_Round TFT_display;
#endif

void takeDisplayMutex();
void giveDisplayMutex();

void displaySetup();

void setClockFace(ClockFace* face);
bool getDisplayTime(struct tm* timeinfo);
void redrawDisplay();

void displayWifiError();
void displaySyncError();
void displayResetQuestion();
void displayWifiSetupInstructions();


#endif
