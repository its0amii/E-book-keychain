// input.cpp — 8-button debounce + event queue
// ============================================================
#include "input.h"

void Input::begin() {
  for (uint8_t i = 0; i < BTN_COUNT; i++) {
    pinMode(pins[i], INPUT_PULLUP);
    pressed[i] = false;
    prevRaw[i] = false;
    debounceTs[i] = 0;
    pressStart[i] = 0;
    longFired[i] = false;
  }
  pending = EVT_NONE;
}

void Input::tick() {
  uint32_t now = millis();
  for (uint8_t i = 0; i < BTN_COUNT; i++) {
    bool raw = (digitalRead(pins[i]) == LOW);   // active-LOW
    if (raw != prevRaw[i]) {
      debounceTs[i] = now;
      prevRaw[i] = raw;
    }
    if ((now - debounceTs[i]) < BTN_DEBOUNCE_MS) continue;

    if (raw != pressed[i]) {
      pressed[i] = raw;
      if (raw) {
        pressStart[i] = now;
        longFired[i] = false;
        // rising edge → emit press event
        switch (i) {
          case BTN_UP:     pending = EVT_UP_PRESS;    break;
          case BTN_DOWN:   pending = EVT_DOWN_PRESS;  break;
          case BTN_LEFT:   pending = EVT_LEFT_PRESS;  break;
          case BTN_RIGHT:  pending = EVT_RIGHT_PRESS; break;
          case BTN_OK:     pending = EVT_OK_PRESS;    break;
          case BTN_BACK:   pending = EVT_BACK_PRESS;  break;
          case BTN_DICT_BTN: pending = EVT_DICT_PRESS; break;
          case BTN_OFF:    pending = EVT_OFF_PRESS;   break;
        }
      } else {
        // falling edge → re-arm
        longFired[i] = false;
      }
    }

    // long-press detection (only OK for now)
    if (raw && !longFired[i] &&
        (now - pressStart[i]) > READER_LONGPRESS_MS) {
      if (i == BTN_OK) {
        longFired[i] = true;
        pending = EVT_OK_LONG;
      }
    }
  }
}

InputEvent Input::consumeEvent() {
  InputEvent e = pending;
  pending = EVT_NONE;
  return e;
}

uint32_t Input::pressDuration(BtnId b) const {
  if (!pressed[b]) return 0;
  return millis() - pressStart[b];
}
