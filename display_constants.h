#ifndef DISPLAY_CONSTANTS_H
#define DISPLAY_CONSTANTS_H

// Display dimensions
#define SCREEN_WIDTH  240
#define SCREEN_HEIGHT 240
#define CENTER_X (SCREEN_WIDTH / 2)
#define CENTER_Y (SCREEN_HEIGHT / 2)

// Clock geometry
#define CLOCK_RADIUS       110
#define HOUR_HAND_LENGTH    50
#define MINUTE_HAND_LENGTH  70
#define SECOND_HAND_LENGTH  85

// Textbox
#define TEXTBOX_WIDTH  140
#define TEXTBOX_HEIGHT  35
#define TEXTBOX_X      ((SCREEN_WIDTH - TEXTBOX_WIDTH) / 2)
#define TEXTBOX_Y      160

// Icons
#define BLINK_INTERVAL_MS 400

#define ICON_WIDTH  24
#define ICON_HEIGHT 24
#define ICON_X      (CENTER_X - (ICON_WIDTH / 2))
#define ICON_Y      (SCREEN_HEIGHT - (TEXTBOX_HEIGHT + TEXTBOX_Y))

#endif
