#ifndef PINS_H
#define PINS_H

// TFT Display pins.
#define PIN_RST 27  // The ESP32 pin GPIO27 connected to the RST pin of the circular TFT display
#define PIN_DC  25  // The ESP32 pin GPIO25 connected to the DC pin of the circular TFT display
#define PIN_CS  26  // The ESP32 pin GPIO26 connected to the CS pin of the circular TFT display

#define BOOT_BUTTON_PIN 0

#if !DISABLE_ENCODER
  // KY-040 Rotary Encoder Pin out
  // Power supply - VCC Required 3.3V
  // GND Required Ground
  // CLK GPIO18 Required - Clock input (any GPIO)
  #define PIN_ENCODER_CLK 32
  // DT GPIO19 Required - Data input (any GPIO)
  #define PIN_ENCODER_DT  33
  // SW GPIO21 Optional - Button input (any GPIO)
  #define PIN_ENCODER_SW  14
#endif

#endif
