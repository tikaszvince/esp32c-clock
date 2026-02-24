#ifndef BUTTON_H
#define BUTTON_H

#include "app_state.h"

void buttonSetup(
  void (*onDoubleClick)()
);
void buttonLoop();

#endif
