#include <math.h>
#include "clock_face_helpers.h"
#include "display.h"
#include "display_constants.h"
#include "icons.h"
#include "app_state.h"

static const int ICON_SIZE = 24;

int collectHandPixels(
  float angleDeg,
  int length,
  int width,
  Pixel* buf,
  int bufSize,
  bool (*clipFn)(int x, int y)
) {
  float rad = angleDeg * PI / 180.0f;
  float perpRad = rad + PI / 2.0f;

  int ex = CENTER_X + (int)(length * cosf(rad));
  int ey = CENTER_Y + (int)(length * sinf(rad));

  int count = 0;
  int half = width / 2;
  for (int i = -half; i <= half; i++) {
    int ox = (int)roundf(i * cosf(perpRad));
    int oy = (int)roundf(i * sinf(perpRad));
    int x0 = CENTER_X + ox, y0 = CENTER_Y + oy;
    int x1 = ex + ox, y1 = ey + oy;

    int dx = abs(x1 - x0), sx = x0 < x1 ? 1 : -1;
    int dy = -abs(y1 - y0), sy = y0 < y1 ? 1 : -1;
    int err = dx + dy;
    while (true) {
      if (!clipFn(x0, y0) && count < bufSize) {
        buf[count++] = {(int16_t)x0, (int16_t)y0};
      }
      if (x0 == x1 && y0 == y1) {
        break;
      }
      int e2 = 2 * err;
      if (e2 >= dy) {
        err += dy;
        x0 += sx;
      }
      if (e2 <= dx) {
        err += dx;
        y0 += sy;
      }
    }
  }
  return count;
}

void drawHandDiff(
  int length,
  int width,
  float newAngle,
  Pixel* lastPixels,
  int& lastCount,
  int bufSize,
  uint16_t color,
  uint16_t backgroundColor,
  bool (*clipFn)(int x, int y)
) {
  Pixel newPixels[bufSize];
  int newCount = collectHandPixels(newAngle, length, width, newPixels, bufSize, clipFn);

  for (int i = 0; i < newCount; i++) {
    TFT_display.drawPixel(newPixels[i].x, newPixels[i].y, color);
  }

  for (int i = 0; i < lastCount; i++) {
    bool found = false;
    for (int j = 0; j < newCount; j++) {
      if (lastPixels[i].x == newPixels[j].x && lastPixels[i].y == newPixels[j].y) {
        found = true;
        break;
      }
    }
    if (!found) {
      TFT_display.drawPixel(lastPixels[i].x, lastPixels[i].y, backgroundColor);
    }
  }

  memcpy(lastPixels, newPixels, newCount * sizeof(Pixel));
  lastCount = newCount;
}

void drawStatusIcons(
  AppState state,
  bool blinkState,
  int wifiX,
  int wifiY,
  int ntpX,
  int ntpY
) {
  bool wifiVisible = true;
  bool wifiOk = true;

  switch (state) {
    case CONNECTED_NOT_SYNCED:
    case CONNECTED_SYNCING:
    case CONNECTED_SYNCED:
      wifiVisible = true;
      wifiOk = true;
      break;
    case CONNECTING:
      wifiVisible = blinkState;
      wifiOk = true;
      break;
    case NOT_CONFIGURED:
    case DISCONNECTED:
      wifiVisible = true;
      wifiOk = false;
      break;
    default:
      wifiVisible = false;
      break;
  }

  if (wifiVisible) {
    TFT_display.drawRGBBitmap(wifiX, wifiY, wifiOk ? IconWifiBitmap : IconWifiOffBitmap, ICON_SIZE, ICON_SIZE);
  }
  else {
    TFT_display.fillRect(wifiX, wifiY, ICON_SIZE, ICON_SIZE, COLOR_BACKGROUND);
  }

  bool syncVisible = (state == CONNECTED_SYNCING || isNtpSyncRequested()) ? blinkState : false;
  if (syncVisible) {
    TFT_display.drawRGBBitmap(ntpX, ntpY, IconSyncBitmap, ICON_SIZE, ICON_SIZE);
  }
  else {
    TFT_display.fillRect(ntpX, ntpY, ICON_SIZE, ICON_SIZE, COLOR_BACKGROUND);
  }
}
