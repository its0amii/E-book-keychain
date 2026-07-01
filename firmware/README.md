# E-Book Keychain — Firmware

A modular C++ firmware for the **ESP32-C6 LCD Development Board**
with 8 navigation buttons and a **CJMCU-4344 (CS4344) I2S stereo DAC**.

Implements the spec in `../ESP32-C6_LCD_Reading_Device_Specification.md`
and follows the design language in `../ui_desinging.md`.

## Folder layout

```
program/
├── platformio.ini             # build config
├── README.md                  # this file
├── include/                   # headers (.h)
│   ├── config.h               # pins, constants
│   ├── theme.h                # colour palette
│   ├── types.h                # Screen / BtnId / state structs
│   ├── ui.h
│   ├── input.h
│   ├── storage.h
│   ├── book_manager.h
│   ├── reader.h
│   ├── dictionary.h
│   ├── audio_player.h
│   ├── wifi_manager.h
│   ├── settings.h
│   └── User_Setup.h           # TFT_eSPI pin map (1.47" 172x320)
├── src/                       # sources (.cpp)
│   ├── main.cpp               # state machine + setup/loop
│   ├── ui.cpp
│   ├── input.cpp
│   ├── storage.cpp
│   ├── book_manager.cpp
│   ├── reader.cpp
│   ├── dictionary.cpp
│   ├── audio_player.cpp
│   ├── wifi_manager.cpp
│   └── settings.cpp
├── scripts/
│   └── copy_user_setup.py     # platformio pre-build
└── data/                      # sample SD content
    ├── books/                 # drop your .txt files here
    └── audio/                 # drop your .mp3 files here
```

## Hardware

| Block         | Pins                                           |
|---------------|------------------------------------------------|
| LCD           | MOSI 6, SCLK 7, CS 14, DC 15, RST 21, BL 22    |
| 8 nav buttons | UP 1, DOWN 2, LEFT 3, RIGHT 10, OK 11, OFF 17, DICT 4, BACK 5 |
| Wake button   | BOOT on GPIO9                                  |
| RGB LED       | GPIO8                                          |
| SD card       | CS 4, MISO 5, MOSI 6, SCLK 7 (shares SPI bus)  |
| CS4344 DAC    | MCLK 0, BCLK 19, WS 20, DOUT 18 (I2S)         |

## Build & flash

```bash
# install PlatformIO
pip install platformio

cd program
pio run -t upload
pio device monitor
```

For the **1.69"** board, edit `platformio.ini` and uncomment
the `[env:esp32c6-lcd-169]` block (and comment out the 1.47" one).

## SD card layout

```
SD root
├── books/                # .txt files for the speed reader
│   └── Harry Potter.txt
├── audio/                # .mp3 / .wav for the audiobook player
│   └── Book1.mp3
├── dict.txt              # one word per record (see below)
├── settings.cfg          # auto-generated
├── progress.cfg          # auto-generated, per-book reading position
└── state.cfg             # auto-generated, last screen
```

`dict.txt` format:

```
robotics
The branch of technology that deals with the design, construction, operation, and application of robots.

engineering
The application of scientific principles to design and build machines.
```

A starter dict with two words is auto-created the first time
`storage.begin()` runs.

## Features

- **Dark UI** matching `ui_desinging.md` — matte background, floating
  rounded cards, orange gradient progress, glassmorphism faked with
  darker overlays.
- **8-button navigation** — UP/DOWN move selection, LEFT/RIGHT
  page-step, OK select/pause, OFF deep-sleep, DICT look up the current
  word, BACK go back.
- **Speed reader** — load a .txt, split into words, advance at
  configurable WPM (100-900, default 250). Position auto-saves on
  BACK or on hold-OK.
- **Dictionary** — DICT while reading looks up the current word
  in `/dict.txt`, with a simple line-wrapped definition view.
- **Audiobook player** — MP3 / WAV from SD, output through the
  CS4344 I2S DAC, with play/pause and progress display.
- **Wi-Fi file sharing** — when enabled, the device spins up an
  AP (`EBook-Keychain` / `12345678`) and serves a tiny web page
  where you can upload .txt / .mp3 / dict.txt. Auto-stops after
  5 minutes of inactivity.
- **Deep sleep** with full state restore — last screen, current
  book, reading position and WPM survive sleep; wake on BOOT.
- **Persistent settings** — brightness, volume, WPM, Wi-Fi state
  saved to `/settings.cfg`.

## Decisions on the spec's "Open Questions"

1. **Graphics library** — TFT_eSPI (matches what we already use).
2. **Audio decoder** — `earlephilhower/ESP8266Audio` (MP3 + WAV).
3. **Load whole book or stream** — load whole book in RAM.
   Books of ~200k words (~1.2 MB) fit easily; bigger books would
   need streaming.
4. **dict.txt format** — word, then 1+ definition lines, then a
   blank line as the record separator (see sample above).
5. **Save reading position per book** — yes, in `/progress.cfg`.
6. **Keep punctuation** — kept attached (per spec).
7. **File sharing delete/rename** — delete is supported in the
   web UI; rename is a future TODO.
8. **Restore last screen** — yes, saved on every state change and
   on entering sleep.
