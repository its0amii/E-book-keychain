// theme.h — colour palette + design tokens
// ============================================================
// Maps the colours from ui_desinging.md into 16-bit RGB565
// (the format TFT_eSPI uses).  All UI code should pull colours
// from this file — never hard-code a hex literal.
// ============================================================
#pragma once
#include <Arduino.h>

// ---------------- Base surfaces ----------------
// #0B0B0F  (page background)   → 0x0841
// #1A1C22  (card surface)      → 0x2945
// #2A2D36  (raised / pressed)  → 0x3186
// #07070A  (vignette edges)    → 0x0841  (same as bg on C6)
// rgba(255,255,255,0.06) divider → 0x4208
static constexpr uint16_t COLOR_BG          = 0x0841;
static constexpr uint16_t COLOR_CARD        = 0x2945;
static constexpr uint16_t COLOR_CARD_RAISED = 0x3186;
static constexpr uint16_t COLOR_DIVIDER     = 0x4208;
static constexpr uint16_t COLOR_VIGNETTE    = 0x0421;

// ---------------- Text ----------------
// #FFFFFF primary   → 0xFFFF
// #9AA0A6 secondary → 0xAD55
// #7E848C muted     → 0x738E
static constexpr uint16_t COLOR_TEXT         = 0xFFFF;
static constexpr uint16_t COLOR_TEXT_DIM     = 0xAD55;
static constexpr uint16_t COLOR_TEXT_MUTED   = 0x738E;
static constexpr uint16_t COLOR_TEXT_INVERT  = 0x0841;  // on orange

// ---------------- Accent (orange system) ----------------
// #FF6A00  (orange)        → 0xFD40  (R=31 G=26 B=0)
// #FF3B00  (deep orange)   → 0xF9E0  (R=31 G=15 B=0)
// #FF9500  (lighter accent)→ 0xFDE0
// #FF3B30  (red / alert)   → 0xF8E6
static constexpr uint16_t COLOR_ACCENT         = 0xFD40;
static constexpr uint16_t COLOR_ACCENT_DEEP    = 0xF9E0;
static constexpr uint16_t COLOR_ACCENT_LIGHT   = 0xFDE0;
static constexpr uint16_t COLOR_DANGER         = 0xF8E6;
static constexpr uint16_t COLOR_OK             = 0x07E0;   // green for status

// ---------------- Progress bar gradient (orange) ----------------
static constexpr uint16_t PROG_GRADIENT[6] = {
  0xF800, 0xF940, 0xFA80, 0xFBC0, 0xFD40, 0xFE60
};

// ---------------- Design tokens ----------------
static constexpr uint8_t  RADIUS_CARD   = 18;   // 18-28px from spec
static constexpr uint8_t  RADIUS_BUTTON = 10;
static constexpr uint8_t  RADIUS_PILL   = 12;
static constexpr uint8_t  CARD_PADDING  = 16;
static constexpr uint8_t  CARD_GAP      = 8;
static constexpr uint8_t  ICON_SIZE     = 16;
static constexpr uint8_t  PROGRESS_H    = 10;
static constexpr uint8_t  PROGRESS_MARK = 4;   // marker width
