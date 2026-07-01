// settings.h — persistent app settings
// ============================================================
#pragma once
#include <Arduino.h>
#include "types.h"

class Settings {
public:
  void load();
  void save();
  AppSettings& mutableRef() { return m_data; }
  const AppSettings& data() const { return m_data; }

private:
  AppSettings m_data;
};

extern Settings settings;
