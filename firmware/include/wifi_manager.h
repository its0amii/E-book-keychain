// wifi_manager.h — AP mode + captive portal for file uploads
// ============================================================
// Spins up a Wi-Fi access point and a tiny HTTP server that
// lets the user upload .txt / .mp3 / dict.txt via the browser.
// When idle for a few minutes, the AP turns itself off to
// save battery.
// ============================================================
#pragma once
#include <Arduino.h>

class WifiManager {
public:
  // Returns true if the AP started successfully.
  bool beginAP();
  void stopAP();

  bool isAPActive() const { return m_active; }

  // Pump the HTTP server.  Call from loop().
  void tick();

private:
  bool   m_active = false;
  uint32_t m_lastActivityMs = 0;
};

extern WifiManager wifiManager;
