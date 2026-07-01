// reader.cpp — speed-reader engine
// ============================================================
#include "reader.h"
#include "storage.h"
#include "config.h"

Reader reader;

void Reader::begin() {
  m_state = ReaderState{};
  m_state.wpm = READER_DEF_WPM;
}

void Reader::close() {
  if (m_wordOff) { delete[] m_wordOff; m_wordOff = nullptr; }
  if (m_wordLen) { delete[] m_wordLen; m_wordLen = nullptr; }
  m_text = "";
  m_state.currentWord = 0;
  m_state.totalWords  = 0;
  m_state.currentBookPath = "";
  m_open = false;
}

bool Reader::open(const String& path) {
  close();
  m_text = storage.readAll(path.c_str());
  if (m_text.length() == 0) return false;
  m_state.currentBookPath = path;
  rebuildIndex();
  if (m_state.totalWords == 0) return false;
  // restore saved position if any
  uint32_t w = 0;
  if (storage.loadProgress(path.c_str(), w)) {
    if (w < m_state.totalWords) m_state.currentWord = w;
  }
  m_open = true;
  return true;
}

// Split the text into words.  Whitespace (space, tab, CR, LF)
// is a separator; punctuation stays attached.
void Reader::rebuildIndex() {
  uint32_t n = m_text.length();
  // first pass: count words
  uint32_t count = 0;
  bool inWord = false;
  for (uint32_t i = 0; i < n; i++) {
    char c = m_text[i];
    bool isWS = (c == ' ' || c == '\t' || c == '\n' || c == '\r');
    if (!isWS && !inWord) { count++; inWord = true; }
    else if (isWS && inWord) { inWord = false; }
  }
  if (count == 0) return;
  m_wordOff = new uint32_t[count];
  m_wordLen = new uint16_t[count];

  // second pass: fill offsets
  uint32_t idx = 0;
  uint32_t start = 0;
  inWord = false;
  for (uint32_t i = 0; i < n; i++) {
    char c = m_text[i];
    bool isWS = (c == ' ' || c == '\t' || c == '\n' || c == '\r');
    if (!isWS && !inWord) { start = i; inWord = true; }
    else if (isWS && inWord) {
      m_wordOff[idx] = start;
      m_wordLen[idx] = (uint16_t)(i - start);
      idx++;
      inWord = false;
    }
  }
  if (inWord) { m_wordOff[idx] = start; m_wordLen[idx] = (uint16_t)(n - start); idx++; }
  m_state.totalWords = count;
  m_state.currentWord = 0;
  m_state.lastTickMs = millis();
}

String Reader::currentWord() const {
  if (!m_open || m_state.currentWord >= m_state.totalWords) return "";
  uint32_t off = m_wordOff[m_state.currentWord];
  uint16_t len = m_wordLen[m_state.currentWord];
  return m_text.substring(off, off + len);
}

void Reader::next() {
  if (!m_open) return;
  if (m_state.currentWord + 1 < m_state.totalWords) m_state.currentWord++;
  m_state.lastTickMs = millis();
}

void Reader::prev() {
  if (!m_open) return;
  if (m_state.currentWord > 0) m_state.currentWord--;
  m_state.lastTickMs = millis();
}

void Reader::goTo(uint32_t idx) {
  if (!m_open) return;
  if (idx >= m_state.totalWords) idx = m_state.totalWords - 1;
  m_state.currentWord = idx;
  m_state.lastTickMs = millis();
}

void Reader::stepWpm(int16_t delta) {
  int32_t w = (int32_t)m_state.wpm + delta;
  if (w < READER_MIN_WPM) w = READER_MIN_WPM;
  if (w > READER_MAX_WPM) w = READER_MAX_WPM;
  m_state.wpm = (uint16_t)w;
}

void Reader::tick() {
  if (!m_open || !m_state.autoAdvance) return;
  uint32_t now = millis();
  if (now - m_state.lastTickMs >= intervalMs()) {
    next();
  }
}

void Reader::savePosition() {
  if (!m_open) return;
  storage.saveProgress(m_state.currentBookPath.c_str(),
                       m_state.currentWord);
}

void Reader::loadPosition() {
  if (!m_open) return;
  uint32_t w = 0;
  if (storage.loadProgress(m_state.currentBookPath.c_str(), w)) {
    goTo(w);
  }
}
