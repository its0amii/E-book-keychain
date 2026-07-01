// ui.cpp — implementation of the UI primitives
// ============================================================
// Follows ui_desinging.md:
//   - dark matte background (#0B0B0F → 0x0841)
//   - floating cards (#1A1C22 → 0x2945) with rounded corners
//   - orange accent (#FF6A00 → 0xFD40)
//   - rounded everything, no sharp edges
//   - fakes glassmorphism with darker overlays + soft dividers
// ============================================================
#include "ui.h"

// ---------------- boot / backlight ----------------
void UI::begin() {
  tft.init();
  tft.setRotation(0);
  tft.fillScreen(COLOR_BG);
  setBacklight(BL_DEFAULT_PCT);
}

void UI::setBacklight(uint8_t pct) {
  if (pct > 100) pct = 100;
  ledcSetup(0, 5000, 8);
  ledcAttachPin(PIN_LCD_BL, 0);
  ledcWrite(0, (pct * 255) / 100);
}

void UI::setBacklightPct(uint8_t pct) {
  setBacklight(pct);
}

void UI::clearScreen() { tft.fillScreen(COLOR_BG); }

// ---------------- background ----------------
void UI::drawBackground() {
  tft.fillScreen(COLOR_BG);
  // fake vignette: a 1-pixel dim border around the screen
  for (uint8_t i = 0; i < 3; i++) {
    tft.drawRect(i, i, SCREEN_W - 2 * i, SCREEN_H - 2 * i, COLOR_VIGNETTE);
  }
}

// ---------------- header ----------------
void UI::drawHeader(const char* title, const char* sub) {
  int16_t y = 0;
  int16_t h = sub ? 38 : 26;
  tft.fillRect(0, y, SCREEN_W, h, COLOR_BG);

  // accent dot (status indicator)
  tft.fillCircle(14, y + 14, 4, COLOR_ACCENT);
  tft.setTextColor(COLOR_TEXT, COLOR_BG);
  tft.setTextSize(2);
  tft.setCursor(28, y + 4);
  tft.print(title);

  if (sub) {
    tft.setTextColor(COLOR_TEXT_DIM, COLOR_BG);
    tft.setTextSize(1);
    tft.setCursor(28, y + 22);
    tft.print(sub);
  }

  // time / status (right aligned)
  tft.setTextColor(COLOR_TEXT_MUTED, COLOR_BG);
  tft.setTextSize(1);
  tft.setCursor(SCREEN_W - 60, y + 8);
  tft.print(LCD_NAME);
}

void UI::drawFooter(const char* hint) {
  if (!hint) return;
  int16_t y = SCREEN_H - 14;
  tft.fillRect(0, y, SCREEN_W, 14, COLOR_BG);
  tft.setTextColor(COLOR_TEXT_MUTED, COLOR_BG);
  tft.setTextSize(1);
  tft.setCursor(6, y + 2);
  tft.print(hint);
}

// ---------------- card ----------------
void UI::drawCard(int16_t x, int16_t y, int16_t w, int16_t h,
                  uint16_t fill, uint16_t border, uint8_t radius,
                  bool raised) {
  uint16_t c = raised ? COLOR_CARD_RAISED : fill;
  // base
  tft.fillRoundRect(x, y, w, h, radius, c);
  // subtle border (1px)
  tft.drawRoundRect(x, y, w, h, radius, border);
  // soft drop shadow
  tft.fillRoundRect(x + 1, y + h, w, 4, radius / 2, dim(COLOR_BG, 200));
  tft.fillRoundRect(x + 2, y + h + 1, w - 2, 2, 1, dim(COLOR_BG, 220));
}

// ---------------- menu list ----------------
void UI::drawMenuList(int16_t x, int16_t y, int16_t w, int16_t h,
                      const char* const* items, uint8_t count,
                      uint8_t selected, uint8_t visibleRows) {
  if (visibleRows == 0) visibleRows = count;
  if (visibleRows > count) visibleRows = count;

  int16_t rowH = h / visibleRows;
  for (uint8_t i = 0; i < visibleRows; i++) {
    int16_t ry = y + i * rowH;
    bool sel = (i == selected);
    uint16_t bg = sel ? COLOR_CARD_RAISED : COLOR_CARD;
    tft.fillRoundRect(x + 2, ry + 1, w - 4, rowH - 2,
                      RADIUS_BUTTON, bg);
    if (sel) {
      // left accent bar
      tft.fillRoundRect(x + 4, ry + 4, 4, rowH - 8, 2, COLOR_ACCENT);
    }
    tft.setTextColor(sel ? COLOR_TEXT : COLOR_TEXT_DIM,
                     sel ? COLOR_CARD_RAISED : COLOR_CARD);
    tft.setTextSize(2);
    tft.setCursor(x + 16, ry + 6);
    tft.print(items[i]);
  }
}

// ---------------- big number ----------------
void UI::drawBigNumber(int16_t x, int16_t y, const char* value,
                       const char* unit, uint16_t col) {
  tft.setTextColor(col, COLOR_BG);
  tft.setTextSize(4);
  tft.setCursor(x, y);
  tft.print(value);
  if (unit) {
    tft.setTextSize(2);
    tft.setTextColor(COLOR_TEXT_DIM, COLOR_BG);
    tft.setCursor(x + 90, y + 20);
    tft.print(unit);
  }
}

// ---------------- progress bar ----------------
uint16_t UI::gradientSample(uint16_t permille) {
  if (permille > 1000) permille = 1000;
  // 6-colour gradient (PROG_GRADIENT) from deep orange → light orange
  uint8_t idx = (permille * 5) / 1000;
  if (idx >= 6) idx = 5;
  return PROG_GRADIENT[idx];
}

void UI::drawProgress(int16_t x, int16_t y, int16_t w, int16_t h,
                      uint16_t permille, bool showMarker, bool showLabel) {
  if (permille > 1000) permille = 1000;

  // track
  tft.fillRoundRect(x, y, w, h, h / 2, COLOR_CARD_RAISED);
  // fill (gradient)
  int16_t fw = (w * permille) / 1000;
  if (fw > 1) {
    for (int16_t dx = 0; dx < fw; dx++) {
      uint16_t p = (dx * 1000) / w;
      tft.drawFastHLine(x + dx, y + 1, 1, gradientSample(p));
    }
  }
  // marker
  if (showMarker && fw > 1 && fw < w - 1) {
    tft.fillRect(x + fw - 1, y - 2, 2, h + 4, COLOR_ACCENT);
  }
  // label
  if (showLabel) {
    char buf[8];
    snprintf(buf, sizeof(buf), "%u%%", permille / 10);
    tft.setTextColor(COLOR_TEXT_MUTED, COLOR_BG);
    tft.setTextSize(1);
    tft.setCursor(x + w + 6, y);
    tft.print(buf);
  }
}

// ---------------- mini line chart ----------------
void UI::drawMiniChart(int16_t x, int16_t y, int16_t w, int16_t h,
                       const float* samples, uint8_t count,
                       uint16_t lineCol) {
  // faint background grid
  for (uint8_t i = 1; i < 4; i++) {
    tft.drawFastHLine(x, y + (h * i) / 4, w, dim(COLOR_TEXT, 240));
  }
  if (count < 2) return;
  int16_t prevX = x, prevY = y + h - 1;
  for (uint8_t i = 0; i < count; i++) {
    int16_t px = x + (w * i) / (count - 1);
    float v = samples[i];
    if (v < 0) v = 0; if (v > 1) v = 1;
    int16_t py = y + h - 1 - (int16_t)(v * (h - 2));
    if (i > 0) tft.drawLine(prevX, prevY, px, py, lineCol);
    prevX = px; prevY = py;
  }
}

// ---------------- three-column stat row ----------------
void UI::drawStatRow(int16_t x, int16_t y, int16_t w,
                     const char* v1, const char* l1,
                     const char* v2, const char* l2,
                     const char* v3, const char* l3) {
  int16_t colW = w / 3;
  const char* vals[3] = {v1, v2, v3};
  const char* labs[3] = {l1, l2, l3};
  for (uint8_t i = 0; i < 3; i++) {
    int16_t cx = x + i * colW;
    if (i > 0) {
      tft.drawFastVLine(cx, y + 2, 24, COLOR_DIVIDER);
    }
    tft.setTextColor(COLOR_TEXT, COLOR_BG);
    tft.setTextSize(2);
    tft.setCursor(cx + 8, y);
    tft.print(vals[i]);
    tft.setTextSize(1);
    tft.setTextColor(COLOR_TEXT_MUTED, COLOR_BG);
    tft.setCursor(cx + 8, y + 18);
    tft.print(labs[i]);
  }
}

// ---------------- round control button ----------------
void UI::drawCircleButton(int16_t cx, int16_t cy, uint8_t r,
                          uint16_t fill, const char* glyph) {
  tft.fillCircle(cx, cy, r, fill);
  tft.setTextColor(COLOR_TEXT, fill);
  tft.setTextSize(2);
  int16_t gw = tft.textWidth(glyph);
  int16_t gh = tft.fontHeight();
  tft.setCursor(cx - gw / 2, cy - gh / 2);
  tft.print(glyph);
}

// ---------------- top progress pill ----------------
void UI::drawPill(int16_t x, int16_t y, int16_t w, uint16_t permille,
                  uint16_t trackCol, uint16_t fillCol) {
  if (permille > 1000) permille = 1000;
  uint8_t h = 4;
  tft.fillRoundRect(x, y, w, h, h / 2, trackCol);
  int16_t fw = (w * permille) / 1000;
  if (fw > 1) tft.fillRoundRect(x, y, fw, h, h / 2, fillCol);
}

// ---------------- 2×4 button bar ----------------
void UI::drawButtonBar(const char* labels[8], const bool pressed[8],
                       const uint16_t colors[8]) {
  int16_t barTop = SCREEN_H - 50;
  int16_t cellW = (SCREEN_W - 8) / 4;
  int16_t cellH = 22;
  tft.fillRect(0, barTop, SCREEN_W, 50, COLOR_BG);
  tft.drawFastHLine(0, barTop, SCREEN_W, COLOR_DIVIDER);

  for (uint8_t i = 0; i < 8; i++) {
    uint8_t row = i / 4;
    uint8_t col = i % 4;
    int16_t x = 4 + col * cellW;
    int16_t y = barTop + 2 + row * cellH;
    uint16_t bg = pressed[i] ? colors[i] : COLOR_CARD;
    uint16_t bd = colors[i];
    tft.fillRoundRect(x + 1, y, cellW - 2, cellH - 2,
                      RADIUS_BUTTON, bg);
    tft.drawRoundRect(x + 1, y, cellW - 2, cellH - 2,
                      RADIUS_BUTTON, bd);
    tft.setTextColor(pressed[i] ? COLOR_TEXT_INVERT : bd,
                     pressed[i] ? colors[i] : COLOR_CARD);
    tft.setTextSize(1);
    int16_t tw = tft.textWidth(labels[i]);
    tft.setCursor(x + (cellW - tw) / 2, y + 5);
    tft.print(labels[i]);
  }
}

// ---------------- sleep screen ----------------
void UI::drawSleepScreen() {
  tft.fillScreen(COLOR_BG);
  tft.setTextColor(COLOR_TEXT_DIM, COLOR_BG);
  tft.setTextSize(2);
  tft.setCursor(40, SCREEN_H / 2 - 10);
  tft.print("z z z");
  tft.setTextSize(1);
  tft.setCursor(20, SCREEN_H / 2 + 20);
  tft.print("Press BOOT to wake");
}

// ---------------- splash ----------------
void UI::drawSplash(const char* title, const char* sub,
                    uint16_t progressPermille) {
  tft.fillScreen(COLOR_BG);
  tft.setTextColor(COLOR_TEXT, COLOR_BG);
  tft.setTextSize(3);
  int16_t tw = tft.textWidth(title);
  tft.setCursor((SCREEN_W - tw) / 2, SCREEN_H / 2 - 30);
  tft.print(title);
  tft.setTextSize(1);
  tft.setTextColor(COLOR_TEXT_DIM, COLOR_BG);
  int16_t sw = tft.textWidth(sub);
  tft.setCursor((SCREEN_W - sw) / 2, SCREEN_H / 2 + 4);
  tft.print(sub);
  // progress bar
  drawProgress(20, SCREEN_H / 2 + 30, SCREEN_W - 40, 8,
               progressPermille, false, false);
}

// ---------------- toast ----------------
void UI::drawToast(const char* text, uint16_t color, uint16_t ms) {
  int16_t w = tft.textWidth(text) + 20;
  int16_t x = (SCREEN_W - w) / 2;
  int16_t y = 32;
  tft.fillRoundRect(x, y, w, 22, RADIUS_BUTTON, color);
  tft.setTextColor(COLOR_TEXT_INVERT, color);
  tft.setTextSize(2);
  tft.setCursor(x + 10, y + 4);
  tft.print(text);
  delay(ms);
  // caller is responsible for re-rendering the screen
}

// ---------------- colour helpers ----------------
uint16_t UI::dim(uint16_t c, uint8_t amount) {
  // amount 0 = original, 255 = pure black
  uint8_t r = ((c >> 11) & 0x1F) * 8;
  uint8_t g = ((c >>  5) & 0x3F) * 4;
  uint8_t b = ( c        & 0x1F) * 8;
  r = (r * (255 - amount)) / 255;
  g = (g * (255 - amount)) / 255;
  b = (b * (255 - amount)) / 255;
  return tft.color565(r, g, b);
}

uint16_t UI::lerp(uint16_t a, uint16_t b, uint8_t t) {
  uint8_t ar = ((a >> 11) & 0x1F) * 8;
  uint8_t ag = ((a >>  5) & 0x3F) * 4;
  uint8_t ab = ( a        & 0x1F) * 8;
  uint8_t br = ((b >> 11) & 0x1F) * 8;
  uint8_t bg = ((b >>  5) & 0x3F) * 4;
  uint8_t bb = ( b        & 0x1F) * 8;
  uint8_t r = ar + ((br - ar) * t) / 255;
  uint8_t g = ag + ((bg - ag) * t) / 255;
  uint8_t bl = ab + ((bb - ab) * t) / 255;
  return tft.color565(r, g, bl);
}
