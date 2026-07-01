// storage.cpp — SD card + persistent settings
// ============================================================
#include "storage.h"

Storage storage;

bool Storage::ensureDir(const char* path) {
  if (!mounted) return false;
  if (SD.exists(path)) return true;
  return SD.mkdir(path);
}

bool Storage::begin() {
  SPI.begin(PIN_SD_SCLK, PIN_SD_MISO, PIN_SD_MOSI, PIN_SD_CS);
  if (!SD.begin(PIN_SD_CS, SPI, SD_SPI_FREQ)) {
    mounted = false;
    return false;
  }
  mounted = true;
  ensureDir(SD_BOOKS_DIR);
  ensureDir(SD_AUDIO_DIR);
  return true;
}

void Storage::listDirFiltered(const char* dir, const char* ext,
                              std::vector<BookEntry>& out, bool audio) {
  out.clear();
  if (!mounted) return;
  File root = SD.open(dir);
  if (!root || !root.isDirectory()) return;
  File f = root.openNextFile();
  while (f) {
    if (!f.isDirectory()) {
      String name(f.name());
      if (name.endsWith(ext)) {
        BookEntry b;
        b.path  = String(dir) + "/" + name.substring(name.lastIndexOf('/') + 1);
        b.title = name.substring(name.lastIndexOf('/') + 1,
                                 name.length() - strlen(ext));
        b.size  = f.size();
        out.push_back(b);
      }
    }
    f = root.openNextFile();
  }
  root.close();
}

bool Storage::listBooks(std::vector<BookEntry>& out) {
  listDirFiltered(SD_BOOKS_DIR, ".txt", out, false);
  return true;
}

bool Storage::listAudio(std::vector<AudioEntry>& out) {
  std::vector<BookEntry> tmp;
  listDirFiltered(SD_AUDIO_DIR, ".mp3", tmp, true);
  for (auto& b : tmp) {
    AudioEntry a;
    a.path  = b.path;
    a.title = b.title;
    out.push_back(a);
  }
  return true;
}

String Storage::readAll(const char* path) {
  String s;
  if (!mounted) return s;
  File f = SD.open(path, FILE_READ);
  if (!f) return s;
  size_t total = f.size();
  s.reserve(total + 1);
  while (f.available()) s += (char)f.read();
  f.close();
  return s;
}

bool Storage::appendKV(const char* key, const char* value) {
  return set(key, value);
}

bool Storage::set(const char* key, const char* value) {
  if (!mounted) return false;
  String path = settingsPath();
  // Read existing, rewrite
  String contents;
  File r = SD.open(path, FILE_READ);
  if (r) {
    while (r.available()) contents += (char)r.read();
    r.close();
  }
  // Replace or append
  String k = String(key) + "=";
  int idx = contents.indexOf(k);
  String line = String(key) + "=" + String(value) + "\n";
  if (idx >= 0) {
    int end = contents.indexOf('\n', idx);
    if (end < 0) end = contents.length();
    contents = contents.substring(0, idx) + line + contents.substring(end + 1);
  } else {
    contents += line;
  }
  File w = SD.open(path, FILE_WRITE);
  if (!w) return false;
  w.print(contents);
  w.close();
  return true;
}

String Storage::get(const char* key, const char* def) {
  if (!mounted) return def;
  File r = SD.open(settingsPath(), FILE_READ);
  if (!r) return def;
  String k = String(key) + "=";
  while (r.available()) {
    String line = r.readStringUntil('\n');
    if (line.startsWith(k)) {
      String v = line.substring(k.length());
      v.trim();
      r.close();
      return v;
    }
  }
  r.close();
  return def;
}

bool Storage::saveProgress(const char* bookPath, uint32_t wordIdx) {
  if (!mounted) return false;
  String line = String(bookPath) + " " + String(wordIdx) + "\n";
  // For simplicity rewrite the whole file.  Book count is small.
  String all;
  File r = SD.open(progressPath(), FILE_READ);
  if (r) { while (r.available()) all += (char)r.read(); r.close(); }

  String key = String(bookPath) + " ";
  int idx = all.indexOf(key);
  if (idx >= 0) {
    int end = all.indexOf('\n', idx);
    if (end < 0) end = all.length();
    all = all.substring(0, idx) + line + all.substring(end + 1);
  } else {
    all += line;
  }
  File w = SD.open(progressPath(), FILE_WRITE);
  if (!w) return false;
  w.print(all);
  w.close();
  return true;
}

bool Storage::loadProgress(const char* bookPath, uint32_t& wordIdx) {
  if (!mounted) return false;
  File r = SD.open(progressPath(), FILE_READ);
  if (!r) return false;
  String key = String(bookPath) + " ";
  while (r.available()) {
    String line = r.readStringUntil('\n');
    if (line.startsWith(key)) {
      String v = line.substring(key.length());
      v.trim();
      wordIdx = v.toInt();
      r.close();
      return true;
    }
  }
  r.close();
  return false;
}

bool Storage::saveState(Screen s, const char* bookPath) {
  if (!mounted) return false;
  File w = SD.open(statePath(), FILE_WRITE);
  if (!w) return false;
  w.printf("screen=%d\n", (int)s);
  if (bookPath) w.printf("book=%s\n", bookPath);
  w.close();
  return true;
}

bool Storage::loadState(Screen& s, String& bookPath) {
  s = SCR_HOME;
  bookPath = "";
  if (!mounted) return false;
  File r = SD.open(statePath(), FILE_READ);
  if (!r) return false;
  while (r.available()) {
    String line = r.readStringUntil('\n');
    if (line.startsWith("screen=")) {
      s = (Screen)line.substring(7).toInt();
    } else if (line.startsWith("book=")) {
      bookPath = line.substring(5);
    }
  }
  r.close();
  return true;
}

void Storage::ensureDictTemplate() {
  if (!mounted) return;
  if (SD.exists(SD_DICT_FILE)) return;
  File f = SD.open(SD_DICT_FILE, FILE_WRITE);
  if (!f) return;
  f.print("robotics\n");
  f.print("The branch of technology that deals with the design, construction, operation, and application of robots.\n\n");
  f.print("engineering\n");
  f.print("The application of scientific principles to design and build machines, structures, and other items.\n\n");
  f.close();
}
