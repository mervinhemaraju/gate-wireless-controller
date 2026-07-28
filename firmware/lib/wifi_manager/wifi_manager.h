#pragma once

// Non-blocking WiFi connect/reconnect with exponential backoff and jitter.
// Only ever compiled for the esp32dev target (nothing under test/ includes
// this), so it's free to use Arduino/WiFi APIs directly - unlike
// gate_control and status_decoder, this module has no native-test
// obligation.

#include <cstdint>

class WifiManager {
public:
  void begin(const char* ssid, const char* password);

  // Call every loop iteration. Never blocks: WiFi.begin() itself is
  // asynchronous: this only polls status and re-attempts on backoff.
  void tick(uint32_t nowMs);

  bool isConnected() const;
  int8_t rssi() const; // 0 if not connected

private:
  const char* ssid_ = nullptr;
  const char* password_ = nullptr;
  uint32_t lastAttemptMs_ = 0;
  uint32_t backoffMs_ = 0;
};
