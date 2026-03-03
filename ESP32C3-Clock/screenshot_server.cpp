#if SCREENSHOT_MODE

#include <WiFi.h>
#include <time.h>
#include <WebServer.h>
#include "screenshot_server.h"
#include "display.h"
#include "display_constants.h"
#include "app_state.h"

static WebServer server(80);

static const int STRIP_COUNT = SCREEN_HEIGHT / CAPTURE_STRIP_HEIGHT;
static const uint32_t IMAGE_SIZE = (uint32_t)SCREEN_WIDTH * SCREEN_HEIGHT * 3;
static const uint32_t FILE_SIZE = 54 + IMAGE_SIZE;

static uint16_t stripBuffer[SCREEN_WIDTH * CAPTURE_STRIP_HEIGHT];
static uint8_t rowBuf[SCREEN_WIDTH * 3];

static void writeLe16(uint8_t* buf, uint16_t val) {
  buf[0] = val & 0xFF;
  buf[1] = (val >> 8) & 0xFF;
}

static void writeLe32(uint8_t* buf, uint32_t val) {
  buf[0] = val & 0xFF;
  buf[1] = (val >> 8) & 0xFF;
  buf[2] = (val >> 16) & 0xFF;
  buf[3] = (val >> 24) & 0xFF;
}

static void handleScreenshot() {
  uint8_t header[54];
  memset(header, 0, sizeof(header));

  header[0] = 'B';
  header[1] = 'M';
  writeLe32(header + 2, FILE_SIZE);
  writeLe32(header + 10, 54);
  writeLe32(header + 14, 40);
  writeLe32(header + 18, SCREEN_WIDTH);
  writeLe32(header + 22, (uint32_t)(int32_t)(-SCREEN_HEIGHT));
  writeLe16(header + 26, 1);
  writeLe16(header + 28, 24);

  server.sendHeader("Content-Disposition", "attachment; filename=screenshot.bmp");
  server.setContentLength(FILE_SIZE);
  server.send(200, "image/bmp", "");
  server.sendContent((const char*)header, 54);

  for (int strip = 0; strip < STRIP_COUNT; strip++) {
    takeDisplayMutex();
    screenshotCaptureStrip(strip * CAPTURE_STRIP_HEIGHT, stripBuffer);
    giveDisplayMutex();

    for (int row = 0; row < CAPTURE_STRIP_HEIGHT; row++) {
      for (int col = 0; col < SCREEN_WIDTH; col++) {
        uint16_t pixel = stripBuffer[row * SCREEN_WIDTH + col];
        rowBuf[col * 3 + 0] = (pixel << 3) & 0xF8;
        rowBuf[col * 3 + 1] = (pixel >> 3) & 0xFC;
        rowBuf[col * 3 + 2] = (pixel >> 8) & 0xF8;
      }
      server.sendContent((const char*)rowBuf, sizeof(rowBuf));
    }
  }

  Serial.println("Screenshot served.");
}

void screenshotServerSetup() {
  server.on("/screenshot", HTTP_GET, handleScreenshot);
  server.begin();
  Serial.print("Screenshot server ready: http://");
  Serial.print(WiFi.localIP());
  Serial.println("/screenshot");
}

void screenshotServerLoop() {
  server.handleClient();
}

#endif
