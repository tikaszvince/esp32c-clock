#ifndef BUTTON_H
#define BUTTON_H

enum ButtonMode { NORMAL, RESET_PENDING };

ButtonMode getButtonMode();
void buttonSetup();
void buttonLoop();

#endif