# ESP32 WiFi Clock

A WiFi-connected clock built on an ESP32 with a 1.28-inch circular TFT display. Time is synchronized via NTP and the timezone is configurable through a web portal. The project is buildable with both Arduino IDE and PlatformIO.

---

## Hardware

### Components

- ESP32 development board (dual-core, 240MHz)
- DIYables 1.28-inch Round Circular TFT LCD Display (GC9A01, 240x240, IPS, SPI)
- KY-040 Rotary Encoder, DIYables or compatible

### Display specification

| Property | Value |
|---|---|
| Display size | 1.28 inches (diagonal) |
| Resolution | 240 x 240 pixels |
| Display type | IPS TFT LCD |
| Driver IC | GC9A01 |
| Interface | 4-wire SPI |
| Operating voltage | 3.3V - 5V |
| Color depth | 65K RGB (16-bit) |
| Active area | 32.4mm diameter (circular) |

### TFT Display Wiring

| Display pin | ESP32 GPIO |
|---|---|
| VCC | 3.3V |
| GND | GND |
| SCL (SCLK) | GPIO 18 |
| SDA (MOSI) | GPIO 23 |
| CS | GPIO 26 |
| DC | GPIO 25 |
| RST | GPIO 27 |

The BOOT button (GPIO 0) built into the ESP32 development board is used for user interaction. No additional hardware is required.

### Encoder wiring

| Encoder pin | ESP32 GPIO |
|---|---|
| GND | GND |
| + | 3.3V (shared with display) |
| CLK | GPIO 32 |
| DT | GPIO 33 |
| SW | GPIO 14 |

The encoder + pin shares the 3.3V rail with the display. Two female Dupont sockets
can be pushed onto the same male pin to achieve this without a breadboard.
The KY-040 module includes its own pull-up resistors on CLK, DT and SW so no
additional pull-ups are needed.

---

## Software dependencies

| Library | Author | Purpose |
|---|---|---|
| DIYables_TFT_Round | DIYables.io | TFT display driver |
| WiFiManager | tzapu | WiFi configuration portal |
| OneButton | Mathias Hertel | Button debounce and gesture detection |
| TzDbLookup | anonymousaga | IANA to POSIX timezone conversion |

---

## Building and flashing

### Arduino IDE

1. Install the libraries listed above via **Sketch -> Include Library -> Manage Libraries**
2. Open the `ESP32C3-Clock/` folder as a sketch
3. Select your ESP32 board under **Tools -> Board**
4. Select the correct COM port under **Tools -> Port**
5. Click **Upload**

### PlatformIO

1. Open the repository root folder in VS Code with the PlatformIO extension installed
2. PlatformIO will read `platformio.ini` and install all dependencies automatically
3. Click **Build** then **Upload** from the PlatformIO toolbar, or run `pio run -t upload` from the terminal

The rotary encoder feature is included by default. To build without it, set the
following flag in `platformio.ini`:
```ini
-DDISABLE_ENCODER=1
```

### Screenshot mode

Screenshot mode is a special build configuration that renders a clock face to a BMP image and serves it over HTTP.
It is useful for capturing reference images of each clock face without needing a camera.

To enable it, set the build flags in `platformio.ini`:
```ini
build_flags =
  -DSCREENSHOT_MODE=1
  -DSCREENSHOT_FACE=CLOCK_FACE_ORBIT
  -DSCREENSHOT_YEAR=2026
  -DSCREENSHOT_MONTH=3
  -DSCREENSHOT_DAY=19
  -DSCREENSHOT_HOUR=10
  -DSCREENSHOT_MIN=10
```

`SCREENSHOT_FACE` accepts any value from the `ClockFaceType` enum in `clock_face_factory.h`. The hardcoded time displayed on the face is set in `ESP32C3-Clock.ino` and `display.cpp` and can be adjusted there before building.

The time displayed on the face is controlled by `SCREENSHOT_YEAR`, `SCREENSHOT_MONTH` (1-based), `SCREENSHOT_DAY`, `SCREENSHOT_HOUR`, and `SCREENSHOT_MIN`. These default to 2026-03-19 at 10:10 if not overridden.

WiFi/NTP, the startup screen, and button handling are all disabled in this mode. The device connects using previously saved WiFi credentials and starts an HTTP server. Once it prints the IP address to the serial console, navigate to: `http://<device-ip>/screenshot`

The response is a 240×240 BMP file. The image is rendered in 16-row strips due to heap constraints, so the request takes several seconds to complete. After downloading, restore the normal build by setting `SCREENSHOT_MODE=0` and flashing again.

---

## Configuration

On first boot the device starts a WiFi access point named `ESP32-Clock` with password `clocksetup`. Connect to it from any device and a configuration portal will open automatically (or navigate to `192.168.4.1`).

The portal allows you to configure:

- **WiFi network** — SSID and password of your home network
- **Timezone** — select from a structured dropdown organized by continent. The selected value is stored as a IANA timezone string and applied to NTP time synchronization
- **NTP server** — defaults to `pool.ntp.org`. Can be changed to any NTP server hostname

Configuration is saved to non-volatile storage and survives power cycles. The portal reopens automatically if the saved WiFi network becomes unreachable for an extended period.

- **Default clock face** — select which clock face is shown on startup. This can   also be changed at any time using the rotary encoder without entering the portal.

---

## Changing the clock face

Rotate the encoder knob to cycle through the available clock faces. Each detent
advances or reverses the selection by one face.

When you rotate to a new face a 20-second grace period begins. A cyan arc is drawn
at the outer edge of the display, draining clockwise as the timer counts down. A
small "Click to save" label is shown near the bottom of the display.

During the grace period:
- **Single click** the encoder button to confirm and save the new face as the
  default. The arc and label disappear and the face remains active after reboot.
- **Do nothing** and the face reverts silently to the previously saved default
  when the timer expires.

The grace period duration is controlled by `FACE_GRACE_PERIOD_MS` in
`timing_constants.h` and defaults to 20 seconds.

---

## Button reference

Two buttons are supported: the rotary encoder button (primary) and the BOOT button
on the ESP32 development board (fallback). Both buttons support the same
interactions. The BOOT button is useful when the encoder is not wired or not
accessible, for example inside a housing.

If the encoder feature is disabled via `DISABLE_ENCODER=1`, only the BOOT button
is active.

### Force a time sync

At any point during normal operation you can trigger an immediate NTP time
synchronization by double-clicking either button. This is useful when you suspect
the displayed time has drifted or after a long period without network connectivity.
The sync icon blinks on the display while synchronization is in progress.

### Selecting a clock face

Rotate the encoder knob to cycle through the available clock faces. Each detent
moves one face forward or backward. The display updates immediately so you can
evaluate each face as you go.

Once you stop rotating, a 20-second grace period begins. A cyan arc appears at the
outer edge of the display and drains away as the timer counts down. A small
"Click to save" label is shown near the bottom of the display.

Single-click the encoder button before the arc runs out to confirm your selection.
The face is saved as the new default and will be restored on every subsequent boot.

If you do not click within the grace period the display silently reverts to the
previously saved face. You can start the process again at any time by rotating
the knob.

The default clock face can also be configured in the WiFiManager portal without
using the encoder. See **Configuration** for details.

### Reset configuration

Hold either button for 5 seconds until the display shows a confirmation prompt.
Then double-click to confirm. This erases all saved settings — WiFi credentials,
timezone, NTP server, and default clock face — and restarts the device into
first-time setup mode.

If you do not confirm within 30 seconds the device cancels the reset and returns
to normal operation automatically.

---

## Power saving

Power save mode reduces energy consumption by disabling the Bluetooth radio permanently and turning off the WiFi radio between NTP synchronizations.

### Bluetooth

Bluetooth is disabled unconditionally at startup since the project does not use it. This is a one-time call during `setup()` and requires no configuration.

### WiFi power save mode

When enabled, the WiFi radio is turned off shortly after each successful NTP sync and brought back up only when the next sync is due. The device keeps accurate time between syncs using the ESP32's internal RTC, which continues running with the radio off.

The device status reflects this with a dedicated `SYNCED_WIFI_OFF` state. Clock faces treat this state as normal operation — no error icons are shown.

This option can be toggled in the WiFi configuration portal under **Power safe mode. Disable networking when not needed**. It defaults to enabled and is saved to non-volatile storage alongside the other settings.

### Power save timing constants

| Constant | Default | Description |
|---|---|---|
| `WIFI_OFF_AFTER_SYNC_MS` | 60000ms | How long WiFi stays on after a successful sync before being turned off |
| `NTP_SYNC_DELAY_MS` | 30000ms | How long to wait after WiFi reconnects before attempting NTP sync, allowing the connection to stabilize |

Both constants are defined in `timing_constants.h`.

### Timing constants

Button timing can be adjusted in `timing_constants.h` if your hardware requires it. Physical buttons vary in their bounce characteristics between boards and components:

| Constant | Default | Description |
|---|---|---|
| `BUTTON_DEBOUNCE_MS` | 40ms | Ignore state changes shorter than this after a press |
| `BUTTON_CLICK_MS` | 400ms | Maximum time between clicks to register as double click |
| `LONG_PRESS_TIME_MS` | 5000ms | Hold duration required to trigger reset-pending |
| `RESET_TIMEOUT_MS` | 30000ms | Time allowed to confirm reset before cancelling |

---

## Architecture

### FreeRTOS tasks

The firmware runs four concurrent tasks:

| Task | Core | Description |
|---|---|---|
| Main loop (Arduino) | Core 1 | Button polling and display redraw every 400ms |
| StartupScreen | Core 1 | Drives the startup animation, terminates itself when initialization is complete |
| NtpTask | Core 0 | Checks for pending or scheduled NTP sync every 10 seconds |
| WifiMonitor | Core 0 | Checks WiFi connection status every 30 seconds, attempts reconnection if disconnected |

The display is protected by a mutex. Any task that writes to the display must acquire it first via `takeDisplayMutex()` and release it via `giveDisplayMutex()`.

The rotary encoder is polled in the main loop via `buttonLoop()` alongside the
BOOT button. No additional FreeRTOS task is created for it.

### ClockFace pattern

The display output is abstracted behind a `ClockFace` interface defined in
`clock_face.h`:
```cpp
class ClockFace {
public:
  virtual void draw(AppState state, bool blinkState, tm timeinfo) = 0;
  virtual void reset() = 0;
  virtual const char* getId() const = 0;
  virtual const char* getName() const = 0;
  virtual bool handlesGracePeriodOverlay() const = 0;
  virtual ~ClockFace() {}
};
```

`redrawDisplay()` in `display.cpp` calls the active face's `draw()` method on
every tick, passing the current app state and a blink signal that toggles every
400ms. The face is responsible for deciding what to draw based on those inputs.

The active face is managed by `face_manager.cpp`, which also owns the grace period
state machine. At startup `setConfiguredClockFace()` loads the saved default face
from non-volatile storage and sets it as the active face. During normal operation
the encoder rotates through faces and the face manager handles the grace period
and revert logic.

`getId()` returns a stable lowercase string key used for persistence, for example
`"classic"` or `"bauhaus_auto"`. This value is written to non-volatile storage
when the user confirms a selection and is used to restore the correct face on
boot. It must not change between firmware versions as doing so would break saved
preferences.

`getName()` returns a human-readable label used in the WiFiManager configuration
form, for example `"Classic"` or `"Bauhaus (auto light/dark)"`.

`handlesGracePeriodOverlay()` controls whether the generic grace period overlay
is drawn on top of the face during a face selection. When it returns `false`,
`redrawDisplay()` draws a cyan draining arc at the outer edge of the display and
a small "Click to save" label after each `draw()` call. When it returns `true`
the face is expected to render its own overlay and `redrawDisplay()` does nothing
extra. Most faces return `false`.

`reset()` is called when the face becomes active, either on first display or when
returning from a full-screen overlay such as the reset confirmation or WiFi setup
screen. Implementations use it to set an internal `_needsFullRedraw` flag that
triggers a complete background repaint on the next `draw()` call.

### Available clock faces

| `ClockFaceType` | ID | Description |
|---|---|---|
| `CLOCK_FACE_CLASSIC` | `classic` | Traditional analog clock with hour and minute hands, tick marks, and a digital seconds readout |
| `CLOCK_FACE_ORBIT` | `orbit` | Concentric arcs showing year/month/day/minute progress with large digital time and date |
| `CLOCK_FACE_BAUHAUS_LIGHT` | `bauhaus_light` | Bauhaus-inspired analog face, light theme |
| `CLOCK_FACE_BAUHAUS_DARK` | `bauhaus_dark` | Bauhaus-inspired analog face, dark theme |
| `CLOCK_FACE_BAUHAUS_AUTO` | `bauhaus_auto` | Switches automatically between light and dark Bauhaus themes at 07:00 and 19:00 |

The factory exposes `getFaceAt()`, `getFaceCount()`, `getIndexById()` and
`getFaceById()` for iterating and looking up faces by index or string ID. All
face instances are static globals — no heap allocation is used.

### Implementing a new clock face

1. Create `clock_face_yourname.h` and `clock_face_yourname.cpp`
2. Inherit from `ClockFace` and implement all pure virtual methods:
```cpp
// clock_face_yourname.h
#ifndef CLOCK_FACE_YOURNAME_H
#define CLOCK_FACE_YOURNAME_H

#include "clock_face.h"
#include "app_state.h"

class ClockFaceYourName : public ClockFace {
public:
  void draw(AppState state, bool blinkState, tm timeinfo) override;
  void reset() override;
  const char* getId() const override;
  const char* getName() const override;
  bool handlesGracePeriodOverlay() const override;
};

#endif
```

3. Add a new entry to the `ClockFaceType` enum in `clock_face_factory.h`:
```cpp
enum ClockFaceType {
  CLOCK_FACE_CLASSIC,
  CLOCK_FACE_YOURNAME,
};
```

4. Add a static instance and a row to the `entries` table in
`clock_face_factory.cpp`:
```cpp
#include "clock_face_yourname.h"

static ClockFaceYourName yourNameFace;

static ClockFaceEntry entries[] = {
  { CLOCK_FACE_CLASSIC,   &classicFace   },
  { CLOCK_FACE_YOURNAME,  &yourNameFace  },
};
```

The `reset()` method is called automatically when the face becomes active.
Use it to set any internal `_needsFullRedraw` flag your face uses to trigger
a complete background repaint on the next `draw()` call.

### Available clock faces

| `ClockFaceType` | Description |
|---|---|
| `CLOCK_FACE_CLASSIC` | Traditional analog clock with hour and minute hands, tick marks, and a digital seconds readout |
| `CLOCK_FACE_ORBIT` | Concentric arcs showing year/month/day/minute progress with large digital time and date |
| `CLOCK_FACE_BAUHAUS_LIGHT` | Bauhaus-inspired analog face, light theme |
| `CLOCK_FACE_BAUHAUS_DARK` | Bauhaus-inspired analog face, dark theme |
| `CLOCK_FACE_BAUHAUS_AUTO` | Switches automatically between light and dark Bauhaus themes at 07:00 and 19:00 |

![Classic](screenshots/clockface_classic.png)
![Orbit](screenshots/clockface_orbit.png)
![Bauhaus light](screenshots/clockface_bauhaus_light.png)
![Bauhaus dark](screenshots/clockface_bauhaus_dark.png)

The active face can be changed at runtime using the rotary encoder. The factory
exposes `getFaceAt()`, `getFaceCount()`, `getIndexById()` and `getFaceById()` for
iterating and looking up faces by index or string ID.

### Implementing a new clock face

1. Create `clock_face_yourname.h` and `clock_face_yourname.cpp`
2. Inherit from `ClockFace` and implement `draw()` and `reset()`

```cpp
// clock_face_yourname.h
#ifndef CLOCK_FACE_YOURNAME_H
#define CLOCK_FACE_YOURNAME_H

#include "clock_face.h"
#include "app_state.h"

class ClockFaceYourName : public ClockFace {
public:
  void draw(AppState state, bool blinkState, tm timeinfo) override;
  void reset() override;
};

#endif
```

3. Add a new entry to the `ClockFaceType` enum in `clock_face_factory.h`:

```cpp
enum ClockFaceType {
  CLOCK_FACE_CLASSIC,
  CLOCK_FACE_YOURNAME,
};
```

4. Add a static instance and a case to `getInstance()` in `clock_face_factory.cpp`:

```cpp
#include "clock_face_yourname.h"

static ClockFaceClassic classicFace;
static ClockFaceYourName yourNameFace;

ClockFace* getInstance(ClockFaceType type) {
  switch (type) {
    case CLOCK_FACE_YOURNAME: return &yourNameFace;
    case CLOCK_FACE_CLASSIC:
    default:                  return &classicFace;
  }
}
```

5. Switch to the new face in `ESP32C3-Clock.ino`:

```cpp
setClockFace(getInstance(CLOCK_FACE_YOURNAME));
```

The `reset()` method is called automatically when returning from a full-screen overlay (reset confirmation or WiFi setup instructions). Use it to set any internal `_needsFullRedraw` flags your face uses to trigger a complete background repaint.

### Key configuration constants

| File | Constant | Description |
|---|---|---|
| `timing_constants.h` | `BLINK_INTERVAL_MS` | Display redraw interval and startup spinner framerate |
| `timing_constants.h` | `NTP_SYNC_INTERVAL_MS` | How often the NTP task triggers an automatic time sync |
| `timing_constants.h` | `RECONNECT_INTERVAL_MS` | Minimum time between WiFi reconnection attempts |
| `timing_constants.h` | `WIFI_MONITOR_CHECK_INTERVAL_MS` | How often the WiFi monitor checks connection status |
| `pins.h` | `PIN_RST`, `PIN_DC`, `PIN_CS` | Display SPI control pins |
| `pins.h` | `BOOT_BUTTON_PIN` | GPIO pin for the user button |
| `config.cpp` | `WIFI_HOTSPOT_SSID` | Access point name shown during first-time setup |
| `config.cpp` | `WIFI_HOTSPOT_PASSWORD` | Access point password during first-time setup |
| `timing_constants.h` | `FACE_GRACE_PERIOD_MS` | How long the user has to confirm a face change before it reverts |
| `timing_constants.h` | `ENCODER_DEBOUNCE_MS` | Minimum time between rotation events to filter encoder noise |
