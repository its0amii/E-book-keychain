// reader.h — speed-reader engine
// ============================================================
// Loads a .txt file, splits it into a flat array of word
// offsets, and serves the current word.  All state lives
// in ReaderState so the engine is fully re-entrant.
// ============================================================
#pragma once
#include <Arduino.h>
#include "types.h"

class Reader {
public:
  void begin();

  // Open and parse a .txt file.  Returns true on success.
  // Splits on whitespace, keeps punctuation attached.
  bool open(const String& path);

  void close();

  bool isOpen() const { return m_open; }

  // Get a copy of the current word (safe to keep around).
  String currentWord() const;

  // Navigation
  void next();   // RIGHT
  void prev();   // LEFT
  void goTo(uint32_t idx);
  void stepWpm(int16_t delta);  // UP/DOWN

  // Auto-advance timer.  Call from loop().
  void tick();

  // Save / restore position to SD card (via Storage).
  void savePosition();
  void loadPosition();

  // State accessors
  uint32_t currentIndex() const { return m_state.currentWord; }
  uint32_t totalWords()   const { return m_state.totalWords; }
  uint16_t wpm()          const { return m_state.wpm; }
  bool     autoAdvance()  const { return m_state.autoAdvance; }
  void     setAutoAdvance(bool a) { m_state.autoAdvance = a; }
  const ReaderState& state() const { return m_state; }
  const String& currentPath() const { return m_state.currentBookPath; }

private:
  bool     m_open = false;
  String   m_text;        // whole book in RAM
  uint32_t* m_wordOff = nullptr;  // start offset of each word
  uint16_t* m_wordLen = nullptr;  // length of each word
  ReaderState m_state;

  void rebuildIndex();
  uint32_t intervalMs() const { return (60UL * 1000UL) / m_state.wpm; }
};

extern Reader reader;
