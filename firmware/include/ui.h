// ui.h — user-interface drawing primitives
// ============================================================
// Every visual element in the design language lives here.
// The functions are stateless — callers pass colours and
// sizes explicitly, so screens can be rebuilt quickly.
// ============================================================
#pragma once
#include <Arduino.h>
#include <TFT_eSPI.h>
#include "config.h"
#include "theme.h"

class UI {
public:
  // Call once from setup().  Initialises the LCD and the
  // backlight PWM.
  void begin();

  // Page-level helpers -------------------------------------------------
  void clearScreen();
  void drawBackground();        // dark fill + soft vignette
  void drawHeader(const char* title, const char* sub = nullptr);
  void drawFooter(const char* hint = nullptr);

  // Cards (the "floating panel" primitive)
  void drawCard(int16_t x, int16_t y, int16_t w, int16_t h,
                uint16_t fill = COLOR_CARD,
                uint16_t border = COLOR_DIVIDER,
                uint8_t radius = RADIUS_CARD,
                bool raised = false);

  // Menu (list inside a card)
  void drawMenuList(int16_t x, int16_t y, int16_t w, int16_t h,
                    const char* const* items, uint8_t count,
                    uint8_t selected, uint8_t visibleRows = 0);

  // Big numeric / text hero block (steps/timer)
  void drawBigNumber(int16_t x, int16_t y, const char* value,
                     const char* unit = nullptr,
                     uint16_t col = COLOR_TEXT);

  // Progress bar with marker and gradient fill
  // 0..1000 (permille for smoother animation)
  void drawProgress(int16_t x, int16_t y, int16_t w, int16_t h,
                    uint16_t permille, bool showMarker = true,
                    bool showLabel = true);

  // Mini line chart (no axes).  `samples` is in 0..1.
  void drawMiniChart(int16_t x, int16_t y, int16_t w, int16_t h,
                     const float* samples, uint8_t count,
                     uint16_t lineCol = COLOR_ACCENT);

  // Three-column stat row at the bottom of a card
  void drawStatRow(int16_t x, int16_t y, int16_t w,
                   const char* v1, const char* l1,
                   const char* v2, const char* l2,
                   const char* v3, const char* l3);

  // Two-button round control (Pause / Stop)
  void drawCircleButton(int16_t cx, int16_t cy, uint8_t r,
                        uint16_t fill, const char* glyph);

  // Top mini progress ring (thin pill)
  void drawPill(int16_t x, int16_t y, int16_t w, uint16_t permille,
                uint16_t trackCol = COLOR_CARD_RAISED,
                uint16_t fillCol  = COLOR_ACCENT_LIGHT);

  // The 2×4 button bar (the "physical keys" at the bottom).
  void drawButtonBar(const char* labels[8], const bool pressed[8],
                     const uint16_t colors[8]);

  // Sleep screen
  void drawSleepScreen();

  // Splash / boot
  void drawSplash(const char* title, const char* sub,
                  uint16_t progressPermille = 0);

  // Toast notification at the top
  void drawToast(const char* text, uint16_t color = COLOR_ACCENT,
                 uint16_t ms = 1500);

  // Colour helpers
  static uint16_t dim(uint16_t c, uint8_t amount);  // 0..255
  static uint16_t lerp(uint16_t a, uint16_t b, uint8_t t);
  static uint16_t gradientSample(uint16_t permille);

  // Backlight control (0..100%) — exposed so main.cpp can dim to 0 on sleep
  void setBacklightPct(uint8_t pct);

  // Direct access to the TFT if a module really needs to
  TFT_eSPI& raw() { return tft; }

private:
  TFT_eSPI tft;
  void     setBacklight(uint8_t pct);
};
