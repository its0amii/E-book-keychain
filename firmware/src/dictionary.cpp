// dictionary.cpp
// ============================================================
#include "dictionary.h"
#include "storage.h"
#include "config.h"
#include <SD.h>

Dictionary dictionary;

void Dictionary::begin() {
  storage.ensureDictTemplate();
}

String Dictionary::normalize(const String& w) const {
  String out;
  out.reserve(w.length());
  for (size_t i = 0; i < w.length(); i++) {
    char c = w[i];
    if ((c >= 'A' && c <= 'Z')) c = c - 'A' + 'a';
    if ((c >= 'a' && c <= 'z') || (c >= '0' && c <= '9')) {
      out += c;
    }
  }
  return out;
}

bool Dictionary::lookup(const String& word, String& definition) {
  definition = "";
  String target = normalize(word);
  if (target.length() == 0) return false;
  if (!storage.isMounted()) return false;

  File f = SD.open(SD_DICT_FILE, FILE_READ);
  if (!f) return false;

  String currentKey;
  String currentDef;
  bool   inDef = false;
  while (f.available()) {
    String line = f.readStringUntil('\n');
    line.trim();
    if (line.length() == 0) {
      // record boundary
      if (currentKey.length() && normalize(currentKey) == target) {
        definition = currentDef;
        f.close();
        return true;
      }
      currentKey = "";
      currentDef = "";
      inDef = false;
      continue;
    }
    if (!inDef) {
      currentKey = line;
      inDef = true;
    } else {
      if (currentDef.length()) currentDef += "\n";
      currentDef += line;
    }
  }
  // tail
  if (currentKey.length() && normalize(currentKey) == target) {
    definition = currentDef;
    f.close();
    return true;
  }
  f.close();
  return false;
}
