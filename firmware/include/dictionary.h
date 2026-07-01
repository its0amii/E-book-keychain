// dictionary.h — look up a word's definition from /dict.txt
// ============================================================
// File format (one record, two lines, blank line separator):
//
//   <word>
//   <definition line 1>
//   <definition line 2 ...>
//
//   <next word>
//   <definition ...>
//
// Lookup is O(N) over the file but our keychain dictionary
// is small (a few thousand entries) so this is fine.
// ============================================================
#pragma once
#include <Arduino.h>

class Dictionary {
public:
  void begin();

  // Returns true and fills `definition` if found.
  // case-insensitive, punctuation-stripped.
  bool lookup(const String& word, String& definition);

  // Helpers used by the UI
  String normalize(const String& w) const;

private:
  String m_definition;
};

extern Dictionary dictionary;
