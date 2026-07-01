// audio_player.cpp — MP3 / WAV playback through CS4344
// ============================================================
// We use the ESP8266Audio library (works fine on ESP32 too).
// I2S is configured for the CS4344 pin map in config.h, and
// the legacy `i2s_set_pin` honours mck_io_num through the
// C6's GPIO matrix.
// ============================================================
#include "audio_player.h"
#include "config.h"
#include <SD.h>
#include <driver/i2s.h>
#include <ESP8266Audio.h>
#include <AudioFileSourceSD.h>
#include <AudioFileSourceID3.h>
#include <AudioGeneratorMP3.h>
#include <AudioGeneratorWAV.h>
#include <AudioOutputI2S.h>

AudioPlayer audioPlayer;

void AudioPlayer::begin() {
  // Configure I2S with MCLK so the CS4344 locks properly
  i2s_config_t cfg = {
    .mode                 = (i2s_mode_t)(I2S_MODE_MASTER | I2S_MODE_TX),
    .sample_rate          = I2S_SAMPLE_RATE,
    .bits_per_sample      = I2S_BITS_PER_SAMPLE_16BIT,
    .channel_format       = I2S_CHANNEL_FMT_RIGHT_LEFT,
    .communication_format = (i2s_comm_format_t)(I2S_COMM_FORMAT_I2S
                                                 | I2S_COMM_FORMAT_I2S_MSB),
    .intr_alloc_flags     = 0,
    .dma_buf_count        = I2S_DMA_COUNT,
    .dma_buf_len          = I2S_DMA_LEN,
    .use_apll             = true,
    .tx_desc_auto_clear   = true,
    .fixed_mclk           = I2S_SAMPLE_RATE * I2S_MCLK_MULT,
  };
  i2s_driver_install(I2S_PORT, &cfg, 0, NULL);
  i2s_pin_config_t pins = {
    .bck_io_num    = PIN_I2S_BCLK,
    .ws_io_num     = PIN_I2S_WS,
    .data_out_num  = PIN_I2S_DOUT,
    .data_in_num   = I2S_PIN_NO_CHANGE,
  };
  pins.mck_io_num = PIN_I2S_MCLK;
  i2s_set_pin(I2S_PORT, &pins);
  i2s_zero_dma_buffer(I2S_PORT);
}

void AudioPlayer::setVolume(float v) {
  if (v < 0) v = 0; if (v > 1) v = 1;
  m_state.volume = v;
  auto out = static_cast<AudioOutputI2S*>(m_out);
  if (out) out->SetGain(v);
}

bool AudioPlayer::playFile(const char* path) {
  stop();
  String p = path;
  if (!SD.exists(p.c_str())) return false;

  auto src = new AudioFileSourceSD(p.c_str());
  auto id3 = new AudioFileSourceID3(src);
  AudioGenerator* gen = nullptr;
  if (p.endsWith(".mp3")) gen = new AudioGeneratorMP3();
  else if (p.endsWith(".wav")) gen = new AudioGeneratorWAV();
  else { delete src; delete id3; return false; }

  auto out = new AudioOutputI2S(I2S_PORT, AudioOutputI2S::EXTERNAL_I2S);
  out->SetPinout(PIN_I2S_BCLK, PIN_I2S_WS, PIN_I2S_DOUT);
  out->SetGain(m_state.volume);

  if (!gen->begin(id3, out)) {
    delete gen; delete id3; delete src; delete out;
    return false;
  }
  m_playlist = id3;
  m_decoder  = gen;
  m_out      = out;
  m_state.currentPath = p;
  m_state.isPlaying    = true;
  m_state.positionMs   = 0;
  m_startedMs          = millis();
  return true;
}

void AudioPlayer::stop() {
  if (m_decoder) {
    static_cast<AudioGenerator*>(m_decoder)->stop();
    delete static_cast<AudioGenerator*>(m_decoder);
    m_decoder = nullptr;
  }
  if (m_playlist) {
    delete static_cast<AudioFileSourceID3*>(m_playlist);
    m_playlist = nullptr;
  }
  if (m_out) {
    delete static_cast<AudioOutputI2S*>(m_out);
    m_out = nullptr;
  }
  m_state.isPlaying = false;
  m_state.positionMs = 0;
}

void AudioPlayer::pause() {
  m_state.isPlaying = false;
  // Generator doesn't have a true pause; we just stop ticking.
}

void AudioPlayer::resume() {
  if (m_decoder) m_state.isPlaying = true;
}

void AudioPlayer::seekRelative(int seconds) {
  // ESP8266Audio doesn't expose clean seeking.  For now we
  // approximate by re-opening the file at offset 0.  Real
  // seeking can be added by wiring AudioFileSource's seek().
  // Track time so the UI can show something sensible.
  m_state.positionMs = (millis() - m_startedMs);
  if (m_state.positionMs > (uint32_t)abs(seconds) * 1000) {
    m_state.positionMs -= abs(seconds) * 1000;
  } else {
    m_state.positionMs = 0;
  }
}

void AudioPlayer::tick() {
  if (!m_state.isPlaying || !m_decoder) return;
  auto gen = static_cast<AudioGenerator*>(m_decoder);
  if (gen->isRunning()) {
    if (!gen->loop()) gen->stop();
  } else {
    stop();
    return;
  }
  m_state.positionMs = millis() - m_startedMs;
}
