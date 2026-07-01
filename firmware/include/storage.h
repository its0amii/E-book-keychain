// storage.h — SD-card abstraction
// ============================================================
// One place to own the SD card and the directory layout
// declared in config.h.  Every other module that needs
// file I/O should go through Storage.
// ============================================================
#pragma once
#include <Arduino.h>
#include <SD.h>
#include <vector>
#include "config.h"
#include "types.h"

class Storage {
public:
  // Returns true on success.  Performs the SD.begin() and
  // creates the expected directory tree if missing.
  bool begin();

  bool isMounted() const { return mounted; }

  // List .txt files in /books; fills `out`.
  bool listBooks(std::vector<BookEntry>& out);

  // List audio files in /audio; fills `out`.
  bool listAudio(std::vector<AudioEntry>& out);

  // Read whole file into a heap String (use sparingly).
  String readAll(const char* path);

  // Persist a key=value line to /settings.cfg.  Used by Settings.
  bool appendKV(const char* key, const char* value);

  // ---------- progress persistence (per-book) ----------
  bool saveProgress(const char* bookPath, uint32_t wordIdx);
  bool loadProgress(const char* bookPath, uint32_t& wordIdx);

  // ---------- last-screen state (deep-sleep restore) ----------
  bool saveState(Screen s, const char* bookPath = nullptr);
  bool loadState(Screen& s, String& bookPath);

  // ---------- generic key/value store helpers ----------
  String get(const char* key, const char* def = "");
  bool   set(const char* key, const char* value);

  // Create the dict file if missing (empty template).
  void ensureDictTemplate();

private:
  bool mounted = false;
  bool ensureDir(const char* path);
  void listDirFiltered(const char* dir, const char* ext,
                       std::vector<BookEntry>& out, bool audio);
  String settingsPath() { return String(SD_SETTINGS); }
  String progressPath() { return String(SD_PROGRESS); }
  String statePath()    { return String(SD_STATE); }
};

extern Storage storage;
