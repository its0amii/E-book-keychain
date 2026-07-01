// config.h — pin map, constants, hardware config
// ============================================================
// All hardware-specific constants live here.  Anything that
// would change if you swapped boards or displays is grouped
// under this single header.
// ============================================================
#pragma once

#include <Arduino.h>

// ---------------- Board / display ----------------
#define BOARD_147  1
#define BOARD_169  2
#ifndef BOARD
  #define BOARD BOARD_147
#endif

#if BOARD == BOARD_169
  #define SCREEN_W    240
  #define SCREEN_H    280
  #define LCD_NAME    "ESP32-C6-LCD-1.69"
#else
  #define SCREEN_W    172
  #define SCREEN_H    320
  #define LCD_NAME    "ESP32-C6-LCD-1.47"
#endif

// ---------------- LCD (factory-wired) ----------------
#define PIN_LCD_MOSI   6
#define PIN_LCD_SCLK   7
#define PIN_LCD_CS    14
#define PIN_LCD_DC    15
#define PIN_LCD_RST   21
#define PIN_LCD_BL    22
#define PIN_RGB_LED    8

// ---------------- 8 navigation buttons ----------------
// See Project_PinMap.md for the rationale.
#define PIN_BTN_UP      1
#define PIN_BTN_DOWN    2
#define PIN_BTN_LEFT    3
#define PIN_BTN_RIGHT  10
#define PIN_BTN_OK     11
#define PIN_BTN_OFF    17
#define PIN_BTN_DICT    4
#define PIN_BTN_BACK    5

// GPIO9 is the factory BOOT button; we also use it as the
// wake source from deep sleep.
#define PIN_BTN_WAKE    9

// ---------------- I2S / CS4344 DAC ----------------
#define PIN_I2S_MCLK    0
#define PIN_I2S_BCLK   19
#define PIN_I2S_WS     20
#define PIN_I2S_DOUT   18
#define I2S_PORT        I2S_NUM_0
#define I2S_SAMPLE_RATE 44100
#define I2S_MCLK_MULT   256
#define I2S_DMA_LEN     64
#define I2S_DMA_COUNT   8

// ---------------- SD card ----------------
#define PIN_SD_CS       4
#define PIN_SD_MISO     5
#define PIN_SD_MOSI     6   // shared with LCD MOSI
#define PIN_SD_SCLK     7   // shared with LCD SCLK
#define SD_SPI_FREQ     20000000

// ---------------- Storage layout (on SD) ----------------
#define SD_BOOKS_DIR   "/books"
#define SD_AUDIO_DIR   "/audio"
#define SD_DICT_FILE   "/dict.txt"
#define SD_SETTINGS    "/settings.cfg"
#define SD_PROGRESS    "/progress.cfg"
#define SD_STATE       "/state.cfg"

// ---------------- Speed reader ----------------
#define READER_MIN_WPM  100
#define READER_MAX_WPM  900
#define READER_DEF_WPM  250
#define READER_LONGPRESS_MS  500   // hold-OK to save position

// ---------------- Audio player ----------------
#define AUDIO_DEF_VOLUME  0.7f

// ---------------- Wi-Fi file sharing ----------------
#define WIFI_AP_SSID    "EBook-Keychain"
#define WIFI_AP_PASS    "12345678"
#define WIFI_AP_CHANNEL 6

// ---------------- Backlight ----------------
#define BL_DEFAULT_PCT  50    // keep ≤ 50% on 1.47" panel

// ---------------- debounce ----------------
#define BTN_DEBOUNCE_MS  25

// ---------------- helpers ----------------
#ifndef MS_TO_TICKS
  #define MS_TO_TICKS(ms)  ((uint32_t)(ms))
#endif
