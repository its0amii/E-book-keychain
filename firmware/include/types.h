// types.h — shared data types and enums
// ============================================================
// Plain-old-data types used across modules.  Keeping them in
// one place avoids circular header dependencies.
// ============================================================
#pragma once
#include <Arduino.h>
#include <vector>

// ---------------- Screens ----------------
enum Screen : uint8_t {
  SCR_BOOT = 0,
  SCR_HOME,
  SCR_EBOOKS,
  SCR_BOOK_LIST,
  SCR_READER,
  SCR_AUDIOBOOKS,
  SCR_AUDIO_LIST,
  SCR_AUDIO_PLAYER,
  SCR_SETTINGS,
  SCR_WIFI_PORTAL,
  SCR_DICT,
  SCR_ABOUT,
  SCR_SLEEP,
  SCR_COUNT
};

// ---------------- Buttons (8 nav keys) ----------------
enum BtnId : uint8_t {
  BTN_UP = 0,
  BTN_DOWN,
  BTN_LEFT,
  BTN_RIGHT,
  BTN_OK,
  BTN_OFF,
  BTN_DICT_BTN,
  BTN_BACK,
  BTN_COUNT
};

// ---------------- Input events ----------------
enum InputEvent : uint8_t {
  EVT_NONE = 0,
  EVT_UP_PRESS,
  EVT_DOWN_PRESS,
  EVT_LEFT_PRESS,
  EVT_RIGHT_PRESS,
  EVT_OK_PRESS,
  EVT_OK_LONG,
  EVT_BACK_PRESS,
  EVT_DICT_PRESS,
  EVT_OFF_PRESS
};

// ---------------- A menu item ----------------
struct MenuItem {
  const char* title;
  Screen      target;
  uint16_t    color;   // accent colour for the row
};

// ---------------- Book entry ----------------
struct BookEntry {
  String path;       // "/books/foo.txt"
  String title;      // file name without extension
  uint32_t size;     // bytes
};

// ---------------- Audiobook entry ----------------
struct AudioEntry {
  String path;       // "/audio/foo.mp3"
  String title;
};

// ---------------- Reader state ----------------
struct ReaderState {
  String  currentBookPath;
  uint32_t currentWord   = 0;     // 0-based word index
  uint32_t totalWords    = 0;
  uint16_t wpm           = READER_DEF_WPM;
  bool     autoAdvance   = false;
  uint32_t lastTickMs    = 0;
};

// ---------------- Player state ----------------
struct PlayerState {
  String  currentPath;
  uint32_t positionMs    = 0;
  float   volume         = AUDIO_DEF_VOLUME;
  bool    isPlaying      = false;
};

// ---------------- Top-level persistent settings ----------------
struct AppSettings {
  uint8_t  brightnessPct = BL_DEFAULT_PCT;
  float    volume        = AUDIO_DEF_VOLUME;
  uint16_t wpm           = READER_DEF_WPM;
  uint8_t  theme         = 0;     // 0 = dark (only one for now)
  bool     wifiEnabled   = false;
};
