#include "clock_face_classic.h"
#include "display.h"
#include "app_state.h"

void ClockFaceClassic::draw(bool blinkState) {
  // Phase 8a: delegates to existing display functions unchanged.
  // Phase 8b will replace this with the new visual design.
  AppState state = getAppState();

  if (state == RESET_PENDING) {
    displayResetQuestion();
    return;
  }

  if (state == NOT_CONFIGURED) {
    displayWifiSetupInstructions();
    return;
  }

  updateClockDisplay();
  updateIcons(blinkState);
}
