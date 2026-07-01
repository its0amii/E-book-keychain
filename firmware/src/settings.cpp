// settings.cpp
// ============================================================
#include "settings.h"
#include "storage.h"

Settings settings;

void Settings::load() {
  m_data = AppSettings{};
  if (!storage.isMounted()) return;
  m_data.brightnessPct = storage.get("brightness").toInt();
  if (m_data.brightnessPct == 0) m_data.brightnessPct = BL_DEFAULT_PCT;
  m_data.volume   = storage.get("volume", "0.7").toFloat();
  m_data.wpm      = storage.get("wpm", "250").toInt();
  m_data.wifiEnabled = storage.get("wifi", "0") == "1";
}

void Settings::save() {
  if (!storage.isMounted()) return;
  storage.set("brightness", String(m_data.brightnessPct).c_str());
  storage.set("volume",     String(m_data.volume, 2).c_str());
  storage.set("wpm",        String(m_data.wpm).c_str());
  storage.set("wifi",       m_data.wifiEnabled ? "1" : "0");
}
