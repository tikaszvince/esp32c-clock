#ifndef BUTTON_H
#define BUTTON_H

enum ButtonMode { NORMAL, RESET_PENDING };

ButtonMode getButtonMode();
void buttonSetup(
  void (*onDoubleClick)(),
  void (*onLongPressStop)()
);
void buttonLoop();

#endif
