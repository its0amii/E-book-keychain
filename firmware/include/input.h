// input.h — 8-button input layer
// ============================================================
// Reads the 8 physical buttons (UP/DOWN/LEFT/RIGHT/OK/OFF/
// DICT/BACK), debounces them, and exposes:
//   - a polled state for short-press events
//   - a long-press detector for the OK button
// All other modules pull InputEvent from this single source.
// ============================================================
#pragma once
#include <Arduino.h>
#include "config.h"
#include "types.h"

class Input {
public:
  void begin();

  // Call from loop() as often as possible.
  // Translates button transitions into InputEvent values
  // and exposes the current pressed-state for live UI feedback.
  void tick();

  // Returns the latest event (clears after read).
  InputEvent consumeEvent();

  // Live state — true while the corresponding button is held.
  bool isPressed(BtnId b) const { return pressed[b]; }

  // Returns the milliseconds since the current press started
  // (or 0 if not pressed).  Used for hold-OK → save position.
  uint32_t pressDuration(BtnId b) const;

private:
  uint8_t      pins[BTN_COUNT] = {
    PIN_BTN_UP, PIN_BTN_DOWN, PIN_BTN_LEFT, PIN_BTN_RIGHT,
    PIN_BTN_OK, PIN_BTN_OFF,  PIN_BTN_DICT, PIN_BTN_BACK
  };
  bool         pressed[BTN_COUNT]  = { false };
  bool         prevRaw[BTN_COUNT]   = { false };
  uint32_t     debounceTs[BTN_COUNT]= { 0 };
  uint32_t     pressStart[BTN_COUNT]= { 0 };
  bool         longFired[BTN_COUNT] = { false };
  InputEvent   pending = EVT_NONE;
};
