// audio_player.h — MP3 / WAV playback through the CS4344 DAC
// ============================================================
// Built on top of the ESP8266Audio library, which decodes
// the compressed stream and pushes PCM out via I2S.  The
// CS4344 DAC turns it into line-level analogue audio.
// ============================================================
#pragma once
#include <Arduino.h>
#include "types.h"

class AudioPlayer {
public:
  void begin();

  // Start streaming a file from SD.  Returns true on success.
  bool playFile(const char* path);
  void stop();
  void pause();
  void resume();

  // 0.0 .. 1.0
  void setVolume(float v);

  // Skip ±seconds (called by DICT+LEFT/RIGHT combo in main).
  void seekRelative(int seconds);

  // Pump the decoder.  Call from loop().
  void tick();

  bool isPlaying() const { return m_state.isPlaying; }
  uint32_t positionMs() const { return m_state.positionMs; }
  const PlayerState& state() const { return m_state; }

private:
  PlayerState m_state;
  // ESP8266Audio objects are forward-declared to keep this
  // header light; the cpp file owns them.
  void* m_playlist   = nullptr;   // AudioFileSourceSD
  void* m_decoder    = nullptr;   // AudioGeneratorMP3
  void* m_out        = nullptr;   // AudioOutputI2S
  uint32_t m_startedMs = 0;
};

extern AudioPlayer audioPlayer;
