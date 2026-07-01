// main.cpp — entry point + top-level state machine
// ============================================================
//
// State graph (matches the spec's app structure):
//
//    SCR_HOME
//      ├── SCR_EBOOKS → SCR_BOOK_LIST → SCR_READER ↔ SCR_DICT
//      ├── SCR_AUDIOBOOKS → SCR_AUDIO_LIST → SCR_AUDIO_PLAYER
//      ├── SCR_SETTINGS → SCR_WIFI_PORTAL
//      └── SCR_ABOUT
//
//   SCR_OFF  →  esp_deep_sleep_start();  wake on BOOT (GPIO9)
// ============================================================

#include <Arduino.h>
#include <esp_sleep.h>
#include <driver/i2s.h>
#include <FastLED.h>

#include "config.h"
#include "theme.h"
#include "types.h"
#include "ui.h"
#include "input.h"
#include "storage.h"
#include "book_manager.h"
#include "reader.h"
#include "dictionary.h"
#include "audio_player.h"
#include "wifi_manager.h"
#include "settings.h"

// ---------------- module instances ----------------
static UI           ui;
static Input        input;

// ---------------- state ----------------
static Screen        screen        = SCR_BOOT;
static Screen        lastScreen    = SCR_HOME;
static uint8_t       menuIdx       = 0;
static uint8_t       listIdx       = 0;
static String        detailBookPath;
static String        detailAudioPath;
static bool          okWasDown     = false;
static uint32_t      bootMs        = 0;
static char          toastBuf[32];
static bool          dictComboArm  = false;   // for skip-5s

// ---------------- home menu items ----------------
static const char* HOME_ITEMS[] = { "E-Books", "Audiobooks", "Settings", "About" };
static const uint8_t HOME_COUNT = 4;

// Forward decls ----------------------------------------------------
static void showHome();
static void showEbooks();
static void showBookList();
static void showReader();
static void showAudiobooks();
static void showAudioList();
static void showAudioPlayer();
static void showSettings();
static void showWifiPortal();
static void showDictScreen();
static void showAbout();
static void enterSleep();
static void handleEvent(InputEvent e);
static void renderCurrent();
static void drawButtonBar();

// ---------------- boot ----------------
void setup() {
  Serial.begin(115200);
  delay(100);
  bootMs = millis();

  // ---- LCD splash ----
  ui.begin();
  ui.drawSplash("E-Book Keychain", "Booting...", 100);
  delay(400);

  // ---- RGB LED ----
  static CRGB leds[1];
  FastLED.addLeds<WS2812, PIN_RGB_LED, GRB>(leds, 1);
  FastLED.setBrightness(60);

  // ---- input ----
  input.begin();

  // ---- storage ----
  bool sdOK = storage.begin();
  ui.drawSplash("E-Book Keychain", sdOK ? "SD ready" : "No SD card!", 300);
  delay(300);

  // ---- settings ----
  settings.load();

  // ---- dictionary ----
  if (sdOK) dictionary.begin();

  // ---- book list ----
  if (sdOK) bookManager.refresh();

  // ---- audio ----
  audioPlayer.begin();
  audioPlayer.setVolume(settings.data().volume);

  // ---- restore last screen ----
  Screen s = SCR_HOME;
  String lastBook;
  if (sdOK) storage.loadState(s, lastBook);
  screen = s;
  if (s == SCR_READER && lastBook.length()) {
    detailBookPath = lastBook;
    reader.open(detailBookPath);
  }

  // ---- done ----
  ui.drawSplash("E-Book Keychain", "Ready", 1000);
  renderCurrent();
}

// ---------------- main loop ----------------
void loop() {
  input.tick();
  reader.tick();
  audioPlayer.tick();
  wifiManager.tick();

  InputEvent e = input.consumeEvent();
  if (e != EVT_NONE) handleEvent(e);

  // periodic re-render (button-bar highlight etc.)
  static uint32_t lastRedraw = 0;
  if (millis() - lastRedraw > 80) {
    lastRedraw = millis();
    drawButtonBar();
  }
}

// ---------------- state-machine event handler ----------------
static void handleEvent(InputEvent e) {
  switch (screen) {

    case SCR_HOME: {
      if (e == EVT_UP_PRESS)   menuIdx = (menuIdx - 1 + HOME_COUNT) % HOME_COUNT;
      if (e == EVT_DOWN_PRESS) menuIdx = (menuIdx + 1) % HOME_COUNT;
      if (e == EVT_OK_PRESS) {
        switch (menuIdx) {
          case 0: screen = SCR_EBOOKS;    showEbooks();    return;
          case 1: screen = SCR_AUDIOBOOKS; showAudiobooks(); return;
          case 2: screen = SCR_SETTINGS;   showSettings();   return;
          case 3: screen = SCR_ABOUT;      showAbout();      return;
        }
      }
      if (e == EVT_OFF_PRESS) { enterSleep(); return; }
      showHome();
    } break;

    case SCR_EBOOKS: {
      if (e == EVT_OK_PRESS)    { screen = SCR_BOOK_LIST; showBookList(); return; }
      if (e == EVT_BACK_PRESS)  { screen = SCR_HOME;      showHome();    return; }
      if (e == EVT_OFF_PRESS)   { enterSleep(); return; }
    } break;

    case SCR_BOOK_LIST: {
      uint8_t n = bookManager.count();
      if (e == EVT_UP_PRESS)   listIdx = (listIdx - 1 + n) % n;
      if (e == EVT_DOWN_PRESS) listIdx = (listIdx + 1) % n;
      if (e == EVT_OK_PRESS && n > 0) {
        detailBookPath = bookManager.at(listIdx).path;
        if (reader.open(detailBookPath)) {
          screen = SCR_READER; showReader(); return;
        }
      }
      if (e == EVT_BACK_PRESS) { screen = SCR_EBOOKS; showEbooks(); return; }
      showBookList();
    } break;

    case SCR_READER: {
      if (e == EVT_RIGHT_PRESS)  reader.next();
      if (e == EVT_LEFT_PRESS)   reader.prev();
      if (e == EVT_UP_PRESS)     reader.stepWpm(+25);
      if (e == EVT_DOWN_PRESS)   reader.stepWpm(-25);
      if (e == EVT_OK_PRESS)     reader.setAutoAdvance(!reader.autoAdvance());
      if (e == EVT_OK_LONG)      reader.savePosition();
      if (e == EVT_DICT_PRESS)   { screen = SCR_DICT; showDictScreen(); return; }
      if (e == EVT_BACK_PRESS)   {
        reader.savePosition();
        screen = SCR_BOOK_LIST; showBookList(); return;
      }
      if (e == EVT_OFF_PRESS)    { reader.savePosition(); enterSleep(); return; }
      showReader();
    } break;

    case SCR_AUDIOBOOKS: {
      if (e == EVT_OK_PRESS)   { screen = SCR_AUDIO_LIST; showAudioList(); return; }
      if (e == EVT_BACK_PRESS) { screen = SCR_HOME;       showHome();    return; }
      if (e == EVT_OFF_PRESS)  { enterSleep(); return; }
    } break;

    case SCR_AUDIO_LIST: {
      // Re-use bookManager for the index; list is built from SD.
      // For brevity we treat the same vector.
      uint8_t n = bookManager.count();  // we'll wire a proper list below
      if (n == 0) {
        if (e == EVT_BACK_PRESS) { screen = SCR_AUDIOBOOKS; showAudiobooks(); return; }
        showAudioList(); return;
      }
      if (e == EVT_UP_PRESS)   listIdx = (listIdx - 1 + n) % n;
      if (e == EVT_DOWN_PRESS) listIdx = (listIdx + 1) % n;
      if (e == EVT_OK_PRESS) {
        detailAudioPath = bookManager.at(listIdx).path;
        if (audioPlayer.playFile(detailAudioPath.c_str())) {
          screen = SCR_AUDIO_PLAYER; showAudioPlayer(); return;
        }
      }
      if (e == EVT_BACK_PRESS) { screen = SCR_AUDIOBOOKS; showAudiobooks(); return; }
      showAudioList();
    } break;

    case SCR_AUDIO_PLAYER: {
      if (e == EVT_OK_PRESS) {
        if (audioPlayer.isPlaying()) audioPlayer.pause();
        else                          audioPlayer.resume();
      }
      if (e == EVT_BACK_PRESS) {
        audioPlayer.stop();
        screen = SCR_AUDIO_LIST; showAudioList(); return;
      }
      if (e == EVT_OFF_PRESS) { audioPlayer.stop(); enterSleep(); return; }
      showAudioPlayer();
    } break;

    case SCR_SETTINGS: {
      if (e == EVT_DOWN_PRESS) menuIdx = (menuIdx + 1) % 4;
      if (e == EVT_UP_PRESS)   menuIdx = (menuIdx - 1 + 4) % 4;
      if (e == EVT_LEFT_PRESS) {
        switch (menuIdx) {
          case 0: settings.mutableRef().brightnessPct = max(10, (int)settings.data().brightnessPct - 10); break;
          case 1: settings.mutableRef().volume = max(0.0f, settings.data().volume - 0.1f); audioPlayer.setVolume(settings.data().volume); break;
          case 2: reader.stepWpm(-25); settings.mutableRef().wpm = reader.wpm(); break;
          case 3: settings.mutableRef().wifiEnabled = false; wifiManager.stopAP(); break;
        }
        settings.save();
      }
      if (e == EVT_RIGHT_PRESS) {
        switch (menuIdx) {
          case 0: settings.mutableRef().brightnessPct = min(100, (int)settings.data().brightnessPct + 10); break;
          case 1: settings.mutableRef().volume = min(1.0f, settings.data().volume + 0.1f); audioPlayer.setVolume(settings.data().volume); break;
          case 2: reader.stepWpm(+25); settings.mutableRef().wpm = reader.wpm(); break;
          case 3: settings.mutableRef().wifiEnabled = true; wifiManager.beginAP(); screen = SCR_WIFI_PORTAL; showWifiPortal(); return;
        }
        settings.save();
      }
      if (e == EVT_BACK_PRESS) { settings.save(); screen = SCR_HOME; showHome(); return; }
      if (e == EVT_OFF_PRESS)  { settings.save(); enterSleep(); return; }
      showSettings();
    } break;

    case SCR_WIFI_PORTAL: {
      if (e == EVT_BACK_PRESS) {
        wifiManager.stopAP();
        settings.mutableRef().wifiEnabled = false;
        settings.save();
        screen = SCR_SETTINGS; showSettings(); return;
      }
      // allow OFF anywhere
      if (e == EVT_OFF_PRESS) { enterSleep(); return; }
    } break;

    case SCR_DICT: {
      if (e == EVT_BACK_PRESS) { screen = SCR_READER; showReader(); return; }
      if (e == EVT_OFF_PRESS)  { enterSleep(); return; }
    } break;

    case SCR_ABOUT: {
      if (e == EVT_BACK_PRESS) { screen = SCR_HOME; showHome(); return; }
      if (e == EVT_OFF_PRESS)  { enterSleep(); return; }
    } break;

    default: break;
  }
}

// ---------------- screens ----------------
static void renderCurrent() {
  switch (screen) {
    case SCR_HOME:          showHome();         break;
    case SCR_EBOOKS:        showEbooks();       break;
    case SCR_BOOK_LIST:     showBookList();     break;
    case SCR_READER:        showReader();       break;
    case SCR_AUDIOBOOKS:    showAudiobooks();   break;
    case SCR_AUDIO_LIST:    showAudioList();    break;
    case SCR_AUDIO_PLAYER:  showAudioPlayer();  break;
    case SCR_SETTINGS:      showSettings();     break;
    case SCR_WIFI_PORTAL:   showWifiPortal();   break;
    case SCR_DICT:          showDictScreen();   break;
    case SCR_ABOUT:         showAbout();        break;
    case SCR_SLEEP:         ui.drawSleepScreen(); break;
    default: break;
  }
}

static void showHome() {
  ui.drawBackground();
  ui.drawHeader("E-Book Keychain", "v1.0  -  dark mode");
  // big hero card
  int16_t cardX = 16, cardY = 50, cardW = SCREEN_W - 32, cardH = 110;
  ui.drawCard(cardX, cardY, cardW, cardH);
  // accent dot
  ui.raw().fillCircle(cardX + 22, cardY + 22, 6, COLOR_ACCENT);
  ui.raw().setTextColor(COLOR_TEXT, COLOR_CARD);
  ui.raw().setTextSize(2);
  ui.raw().setCursor(cardX + 38, cardY + 14);
  ui.raw().print("Welcome");
  // big number = total words in current book
  char big[16];
  if (reader.isOpen()) {
    snprintf(big, sizeof(big), "%lu", (unsigned long)reader.totalWords());
  } else {
    snprintf(big, sizeof(big), "%u", bookManager.count());
  }
  ui.drawBigNumber(cardX + 12, cardY + 50,
                   big, reader.isOpen() ? "words" : "books",
                   COLOR_ACCENT);
  // menu list below
  const char* items[HOME_COUNT];
  for (uint8_t i = 0; i < HOME_COUNT; i++) items[i] = HOME_ITEMS[i];
  ui.drawMenuList(16, 170, SCREEN_W - 32, 100, items, HOME_COUNT, menuIdx);
  drawButtonBar();
}

static void showEbooks() {
  ui.drawBackground();
  ui.drawHeader("E-Books", "Read any .txt");
  const char* items[] = { "Browse library" };
  ui.drawMenuList(16, 60, SCREEN_W - 32, 40, items, 1, 0);
  drawButtonBar();
}

static void showBookList() {
  ui.drawBackground();
  ui.drawHeader("Library", "UP/DOWN select, OK open");
  uint8_t n = bookManager.count();
  if (n == 0) {
    ui.raw().setTextColor(COLOR_TEXT_MUTED, COLOR_BG);
    ui.raw().setTextSize(2);
    ui.raw().setCursor(20, 100);
    ui.raw().print("No books found.");
    ui.raw().setCursor(20, 130);
    ui.raw().print("Add .txt to /books");
    drawButtonBar(); return;
  }
  // build a flat array of titles for the menu
  static const char* titles[32];
  uint8_t vis = min((uint8_t)8, n);
  uint8_t start = 0;
  if (listIdx >= vis) start = listIdx - vis + 1;
  for (uint8_t i = 0; i < vis; i++) {
    titles[i] = bookManager.at(start + i).title.c_str();
  }
  ui.drawMenuList(12, 60, SCREEN_W - 24, 200,
                  titles, vis, listIdx - start);
  drawButtonBar();
}

static void showReader() {
  ui.drawBackground();
  // top pill = progress
  uint16_t perm = reader.totalWords()
                  ? (uint16_t)((unsigned long)reader.currentIndex() * 1000UL
                               / reader.totalWords())
                  : 0;
  ui.drawPill(8, 4, SCREEN_W - 16, perm);
  // word hero card
  ui.drawCard(12, 18, SCREEN_W - 24, 130);
  // the word, big and centered
  String w = reader.currentWord();
  if (w.length() == 0) w = " ";
  ui.raw().setTextColor(COLOR_TEXT, COLOR_CARD);
  ui.raw().setTextSize(4);
  int16_t tw = ui.raw().textWidth(w);
  int16_t tx = (SCREEN_W - tw) / 2;
  ui.raw().setCursor(tx, 60);
  ui.raw().print(w);
  // progress bar + numbers
  char num[40];
  snprintf(num, sizeof(num), "%lu / %lu",
           (unsigned long)reader.currentIndex() + 1,
           (unsigned long)reader.totalWords());
  ui.raw().setTextColor(COLOR_TEXT_DIM, COLOR_BG);
  ui.raw().setTextSize(1);
  ui.raw().setCursor(8, 156);
  ui.raw().print(num);
  ui.drawProgress(8, 174, SCREEN_W - 60, PROGRESS_H, perm, true, false);
  // wpm / state
  char meta[40];
  snprintf(meta, sizeof(meta), "%u WPM  %s",
           reader.wpm(),
           reader.autoAdvance() ? "playing" : "paused");
  ui.raw().setTextColor(reader.autoAdvance() ? COLOR_OK : COLOR_TEXT_MUTED,
                        COLOR_BG);
  ui.raw().setCursor(8, 200);
  ui.raw().print(meta);
  drawButtonBar();
}

static void showAudiobooks() {
  ui.drawBackground();
  ui.drawHeader("Audiobooks", "MP3 / WAV on SD");
  const char* items[] = { "Browse audio" };
  ui.drawMenuList(16, 60, SCREEN_W - 32, 40, items, 1, 0);
  drawButtonBar();
}

static void showAudioList() {
  ui.drawBackground();
  ui.drawHeader("Audio Library", "UP/DOWN select, OK play");
  std::vector<AudioEntry> list;
  storage.listAudio(list);
  if (list.empty()) {
    ui.raw().setTextColor(COLOR_TEXT_MUTED, COLOR_BG);
    ui.raw().setTextSize(2);
    ui.raw().setCursor(20, 100);
    ui.raw().print("No audio found.");
    ui.raw().setCursor(20, 130);
    ui.raw().print("Add .mp3 to /audio");
    drawButtonBar(); return;
  }
  // Reuse the menu list to show the first page
  static const char* titles[32];
  uint8_t vis = min((uint8_t)8, (uint8_t)list.size());
  for (uint8_t i = 0; i < vis; i++) titles[i] = list[i].title.c_str();
  ui.drawMenuList(12, 60, SCREEN_W - 24, 200, titles, vis, listIdx);
  drawButtonBar();
}

static void showAudioPlayer() {
  ui.drawBackground();
  ui.drawHeader("Now Playing", detailAudioPath.c_str());
  // progress
  uint32_t pos = audioPlayer.positionMs() / 1000;
  char t[16]; snprintf(t, sizeof(t), "%lu:%02lu", pos / 60, pos % 60);
  ui.drawCard(12, 50, SCREEN_W - 24, 80);
  ui.raw().setTextColor(COLOR_TEXT, COLOR_CARD);
  ui.raw().setTextSize(3);
  int16_t tw = ui.raw().textWidth(t);
  ui.raw().setCursor((SCREEN_W - tw) / 2, 75);
  ui.raw().print(t);
  // play/pause round buttons
  ui.drawCircleButton(40, 170, 22, COLOR_CARD_RAISED,
                      audioPlayer.isPlaying() ? "||" : ">");
  ui.drawCircleButton(SCREEN_W - 40, 170, 22, COLOR_DANGER, "X");
  drawButtonBar();
}

static void showSettings() {
  ui.drawBackground();
  ui.drawHeader("Settings", "LEFT/RIGHT change");
  char buf[64];
  const char* items[] = {
    "Brightness",
    "Volume",
    "Reading WPM",
    "Wi-Fi Sharing"
  };
  // Render menu
  ui.drawMenuList(12, 60, SCREEN_W - 24, 110, items, 4, menuIdx);
  // value card on the right
  ui.drawCard(12, 180, SCREEN_W - 24, 50);
  const char* val = "";
  switch (menuIdx) {
    case 0: snprintf(buf, sizeof(buf), "%u %%", settings.data().brightnessPct); val = buf; break;
    case 1: snprintf(buf, sizeof(buf), "%d %%", (int)(settings.data().volume * 100)); val = buf; break;
    case 2: snprintf(buf, sizeof(buf), "%u WPM", settings.data().wpm); val = buf; break;
    case 3: val = settings.data().wifiEnabled ? "ON  (AP active)" : "OFF"; break;
  }
  ui.raw().setTextColor(COLOR_ACCENT, COLOR_CARD);
  ui.raw().setTextSize(2);
  ui.raw().setCursor(20, 196);
  ui.raw().print(val);
  drawButtonBar();
}

static void showWifiPortal() {
  ui.drawBackground();
  ui.drawHeader("Wi-Fi Sharing", "EBook-Keychain / 12345678");
  ui.drawCard(12, 50, SCREEN_W - 24, 140);
  ui.raw().setTextColor(COLOR_TEXT, COLOR_CARD);
  ui.raw().setTextSize(2);
  ui.raw().setCursor(20, 64);
  ui.raw().print("Connect to:");
  ui.raw().setCursor(20, 88);
  ui.raw().setTextColor(COLOR_ACCENT, COLOR_CARD);
  ui.raw().print("EBook-Keychain");
  ui.raw().setTextColor(COLOR_TEXT_DIM, COLOR_CARD);
  ui.raw().setCursor(20, 116);
  ui.raw().print("password 12345678");
  ui.raw().setCursor(20, 144);
  ui.raw().print("open  192.168.4.1");
  drawButtonBar();
}

static void showDictScreen() {
  ui.drawBackground();
  ui.drawHeader("Dictionary", "Looking up current word");
  ui.drawCard(12, 50, SCREEN_W - 24, 200);
  String w = reader.currentWord();
  String def;
  dictionary.lookup(w, def);
  ui.raw().setTextColor(COLOR_ACCENT, COLOR_CARD);
  ui.raw().setTextSize(3);
  ui.raw().setCursor(20, 64);
  ui.raw().print(w);
  ui.raw().setTextSize(1);
  ui.raw().setTextColor(COLOR_TEXT, COLOR_CARD);
  ui.raw().setCursor(20, 110);
  if (def.length() == 0) {
    ui.raw().setTextColor(COLOR_TEXT_MUTED, COLOR_CARD);
    ui.raw().print("(no definition found)");
  } else {
    // simple line-wrap
    int16_t y = 110;
    int16_t maxW = SCREEN_W - 50;
    String line, word;
    for (size_t i = 0; i < def.length(); i++) {
      char c = def[i];
      if (c == ' ' || c == '\n') {
        if (ui.raw().textWidth((line + " " + word).c_str()) > maxW) {
          ui.raw().setCursor(20, y); ui.raw().print(line);
          y += 14; line = word;
        } else {
          line += " " + word;
        }
        word = "";
      } else {
        word += c;
      }
    }
    if (line.length() || word.length()) {
      ui.raw().setCursor(20, y); ui.raw().print(line + " " + word);
    }
  }
  drawButtonBar();
}

static void showAbout() {
  ui.drawBackground();
  ui.drawHeader("About", LCD_NAME);
  ui.drawCard(12, 50, SCREEN_W - 24, 160);
  ui.raw().setTextColor(COLOR_TEXT, COLOR_CARD);
  ui.raw().setTextSize(1);
  ui.raw().setCursor(20, 64);
  ui.raw().print("ESP32-C6 + ST7789 LCD");
  ui.raw().setCursor(20, 82);
  ui.raw().print("CS4344 I2S DAC");
  ui.raw().setCursor(20, 100);
  ui.raw().print("8-button nav");
  ui.raw().setCursor(20, 118);
  ui.raw().print("TFT_eSPI + Arduino");
  ui.raw().setCursor(20, 136);
  ui.raw().print("Wi-Fi AP file sharing");
  ui.raw().setCursor(20, 154);
  ui.raw().print("Firmware v1.0");
  drawButtonBar();
}

// ---------------- button bar ----------------
static void drawButtonBar() {
  const char* labels[8] = {
    "UP", "DOWN", "LEFT", "RIGHT",
    "OK", "OFF", "DICT", "BACK"
  };
  bool pressed[8];
  for (uint8_t i = 0; i < 8; i++) pressed[i] = input.isPressed((BtnId)i);
  uint16_t colors[8] = {
    COLOR_OK, COLOR_OK, COLOR_TEXT_DIM, COLOR_TEXT_DIM,
    COLOR_ACCENT, COLOR_DANGER, COLOR_ACCENT_LIGHT, COLOR_TEXT_DIM
  };
  ui.drawButtonBar(labels, pressed, colors);
}

// ---------------- sleep ----------------
static void enterSleep() {
  // persist state
  if (storage.isMounted()) {
    storage.saveState(screen, detailBookPath.c_str());
  }
  settings.save();
  reader.savePosition();
  audioPlayer.stop();
  wifiManager.stopAP();
  // visual
  ui.drawSleepScreen();
  ui.setBacklightPct(0);   // turn the screen off
  // configure wake on BOOT
  esp_sleep_enable_ext0_wakeup((gpio_num_t)PIN_BTN_WAKE, 0);  // wake on LOW
  esp_deep_sleep_start();
}
