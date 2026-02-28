#ifndef CLOCK_FACE_CLASSIC_H
#define CLOCK_FACE_CLASSIC_H

#include "clock_face.h"
#include "app_state.h"


class ClockFaceClassic : public ClockFace {
public:
  void draw(bool blinkState) override;

private:
  bool _needsFullRedraw = true;
  char _lastText[16] = "";
  void drawBackground();
  void drawTicks();
  void drawTextBoxFrame();
  void clearTextBoxContent();
  void drawTextBoxContent(AppState state);
  void drawIcons(AppState state, bool blinkState);
  void drawHands();
  void drawResetQuestion();
  void drawWifiSetupInstructions();
};

#endif
