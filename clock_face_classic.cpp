#include "clock_face_classic.h"
#include "display.h"
#include "app_state.h"
#include "display_constants.h"
#include "icons.h"

// Geometry
static const uint8_t CLOCK_RADIUS = 120;
static const uint8_t TICK_LENGTH_MAIN = 14;
static const uint8_t TICK_LENGTH_MINOR = 8;
static const uint8_t HOUR_HAND_LENGTH = 50;
static const uint8_t HOUR_HAND_WIDTH = 5;
static const uint8_t MINUTE_HAND_LENGTH = 93;
static const uint8_t MINUTE_HAND_WIDTH = 3;
static const uint8_t CENTER_DOT_RING = 8;
static const uint8_t CENTER_DOT_RADIUS = 5;

// Textbox
static const uint8_t TEXTBOX_WIDTH = 140;
static const uint8_t TEXTBOX_HEIGHT = 35;
static const uint8_t TEXTBOX_Y = 160;
static const uint8_t TEXTBOX_X = (SCREEN_WIDTH - TEXTBOX_WIDTH) / 2;

// Icons
static const uint8_t ICON_SIZE = 24;
static const uint8_t ICON_GAP = 6;
static const uint8_t ICON_WIFI_X = 34;
static const uint8_t ICON_WIFI_Y = 34;
static const uint8_t ICON_NTP_X = SCREEN_WIDTH - ICON_WIFI_X - ICON_SIZE;
static const uint8_t ICON_NTP_Y = ICON_WIFI_Y;

void ClockFaceClassic::draw(bool blinkState) {
  AppState state = getAppState();

  if (state == RESET_PENDING) {
    if (!_needsFullRedraw) {
      drawResetQuestion();
      _needsFullRedraw = true;
    }
    return;
  }

  if (state == NOT_CONFIGURED) {
    if (!_needsFullRedraw) {
      drawWifiSetupInstructions();
      _needsFullRedraw = true;
    }
    return;
  }

  if (_needsFullRedraw) {
    drawBackground();
    drawTicks();
    _needsFullRedraw = false;
  }

  drawIcons(state, blinkState);
  drawTextBoxContent(state);
  drawHands();
}

void ClockFaceClassic::drawBackground() {
  TFT_display.fillScreen(COLOR_BACKGROUND);
}

void ClockFaceClassic::drawTicks() {
  for (int i = 0; i < 12; i++) {
    float angle = (i * 30 - 90) * PI / 180.0f;
    bool isMain = (i % 3 == 0);
    int len = isMain ? TICK_LENGTH_MAIN : TICK_LENGTH_MINOR;

    int x1 = CENTER_X + (int)(CLOCK_RADIUS * cosf(angle));
    int y1 = CENTER_Y + (int)(CLOCK_RADIUS * sinf(angle));
    int x2 = CENTER_X + (int)((CLOCK_RADIUS - len) * cosf(angle));
    int y2 = CENTER_Y + (int)((CLOCK_RADIUS - len) * sinf(angle));

    if (isMain) {
      TFT_display.drawLine(x1, y1, x2, y2, COLOR_CLOCKFACE);
      TFT_display.drawLine(x1 + 1, y1, x2 + 1, y2, COLOR_CLOCKFACE);
      TFT_display.drawLine(x1, y1 + 1, x2, y2 + 1, COLOR_CLOCKFACE);
    } else {
      TFT_display.drawLine(x1, y1, x2, y2, COLOR_CLOCKFACE);
    }
    yield();
  }
}

void ClockFaceClassic::drawResetQuestion() {
  displayResetQuestion();
}

void ClockFaceClassic::drawWifiSetupInstructions() {
  displayWifiSetupInstructions();
}

void ClockFaceClassic::drawTextBoxFrame() {
  // TFT_display.fillRect(TEXTBOX_X, TEXTBOX_Y, TEXTBOX_WIDTH, TEXTBOX_HEIGHT, COLOR_BACKGROUND);
  TFT_display.drawRect(TEXTBOX_X, TEXTBOX_Y, TEXTBOX_WIDTH, TEXTBOX_HEIGHT, COLOR_CLOCKFACE);
}

void ClockFaceClassic::drawTextBoxContent(AppState state) {
  char text[16] = "";

  if (isStatusTextActive()) {
    strncpy(text, getStatusText(), sizeof(text) - 1);
  }
  else {
    switch (state) {
      case CONNECTED_SYNCED: {
        struct tm timeinfo;
        if (getLocalTime(&timeinfo)) {
          sprintf(text, "%02d:%02d:%02d", timeinfo.tm_hour, timeinfo.tm_min, timeinfo.tm_sec);
        }
        else {
          strcpy(text, "--:--:--");
        }
        break;
      }
      case CONNECTED_NOT_SYNCED:
        strcpy(text, "No NTP sync");
        break;
      case CONNECTED_SYNCING:
        strcpy(text, "Syncing...");
        break;
      case CONNECTING:
        strcpy(text, "Connecting");
        break;
      case DISCONNECTED:
        strcpy(text, "No WiFi");
        break;
      default:
        break;
    }
  }

  if (strlen(text) != strlen(_lastText)) {
    TFT_display.fillRect(TEXTBOX_X + 1, TEXTBOX_Y + 1, TEXTBOX_WIDTH - 2, TEXTBOX_HEIGHT - 2, COLOR_BACKGROUND);
  }

  if (strlen(text) == 0) {
    return;
  }

  strncpy(_lastText, text, sizeof(_lastText) - 1);

  TFT_display.setTextColor(COLOR_YELLOW, COLOR_BACKGROUND);
  TFT_display.setTextSize(2);
  int textX = (SCREEN_WIDTH - strlen(text) * 12) / 2;
  TFT_display.setCursor(textX, TEXTBOX_Y + 10);
  TFT_display.print(text);
}

void ClockFaceClassic::drawIcons(AppState state, bool blinkState) {
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
    TFT_display.drawRGBBitmap(ICON_WIFI_X, ICON_WIFI_Y, wifiOk ? IconWifiBitmap : IconWifiOffBitmap, ICON_SIZE, ICON_SIZE);
  }
  else {
    TFT_display.fillRect(ICON_WIFI_X, ICON_WIFI_Y, ICON_SIZE, ICON_SIZE, COLOR_BACKGROUND);
  }

  bool syncVisible = (state == CONNECTED_SYNCING) ? blinkState : false;
  if (syncVisible) {
    TFT_display.drawRGBBitmap(ICON_NTP_X, ICON_NTP_Y, IconSyncBitmap, ICON_SIZE, ICON_SIZE);
  }
  else {
    TFT_display.fillRect(ICON_NTP_X, ICON_NTP_Y, ICON_SIZE, ICON_SIZE, COLOR_BACKGROUND);
  }
}

static void drawSingleHand(float angleDeg, int length, int width, uint16_t color) {
  float rad = angleDeg * PI / 180.0f;
  float perpRad = rad + PI / 2.0f;

  int ex = CENTER_X + (int)(length * cosf(rad));
  int ey = CENTER_Y + (int)(length * sinf(rad));

  int half = width / 2;
  for (int i = -half; i <= half; i++) {
    int ox = (int)roundf(i * cosf(perpRad));
    int oy = (int)roundf(i * sinf(perpRad));
    TFT_display.drawLine(CENTER_X + ox, CENTER_Y + oy, ex + ox, ey + oy, color);
  }
}

float roundAngle(float x) {
  return std::floor((x * 10) + 0.5) / 10;
}

void ClockFaceClassic::drawHands() {
  struct tm timeinfo;
  if (!getLocalTime(&timeinfo)) {
    return;
  }

  float hourAngle = roundAngle((timeinfo.tm_hour % 12) * 30.0f + timeinfo.tm_min * 0.5f);
  float minuteAngle = roundAngle(timeinfo.tm_min * 6.0f + timeinfo.tm_sec * 0.1f);
  bool redrawCenter = false;

  if (hourAngle != _lastHourAngle) {
    drawSingleHand(_lastHourAngle, HOUR_HAND_LENGTH, HOUR_HAND_WIDTH, COLOR_BACKGROUND);
    drawSingleHand(hourAngle, HOUR_HAND_LENGTH, HOUR_HAND_WIDTH, COLOR_CLOCKFACE);
    _lastHourAngle = hourAngle;
    redrawCenter = true;
  }

  if (minuteAngle != _lastMinuteAngle) {
    drawSingleHand(_lastMinuteAngle, MINUTE_HAND_LENGTH, MINUTE_HAND_WIDTH, COLOR_BACKGROUND);
    drawSingleHand(minuteAngle, MINUTE_HAND_LENGTH, MINUTE_HAND_WIDTH, COLOR_MINUTE_HAND);
    _lastMinuteAngle = minuteAngle;
    redrawCenter = true;
  }

  if (redrawCenter) {  
    // Center dot always drawn last to cover hand overlap
    TFT_display.fillCircle(CENTER_X, CENTER_Y, CENTER_DOT_RING, COLOR_BACKGROUND);
    TFT_display.fillCircle(CENTER_X, CENTER_Y, CENTER_DOT_RADIUS, COLOR_CLOCKFACE);
  }
}
