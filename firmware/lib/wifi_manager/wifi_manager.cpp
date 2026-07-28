#include "wifi_manager.h"

#include <WiFi.h>

#include "config.h"
#include "logging.h"

void WifiManager::begin(const char* ssid, const char* password) {
  ssid_ = ssid;
  password_ = password;
  backoffMs_ = WIFI_BACKOFF_INITIAL_MS;
  lastAttemptMs_ = 0;
  WiFi.mode(WIFI_STA);
}

void WifiManager::tick(uint32_t nowMs) {
  if (WiFi.status() == WL_CONNECTED) {
    backoffMs_ = WIFI_BACKOFF_INITIAL_MS;
    return;
  }

  if ((nowMs - lastAttemptMs_) >= backoffMs_) {
    LOG_INFO("wifi: connecting to %s", ssid_);
    WiFi.begin(ssid_, password_);
    lastAttemptMs_ = nowMs;

    uint32_t jitter = random(0, backoffMs_ / 4 + 1);
    uint32_t doubled = backoffMs_ * 2;
    if (doubled > WIFI_BACKOFF_MAX_MS) {
      doubled = WIFI_BACKOFF_MAX_MS;
    }
    backoffMs_ = doubled + jitter;
    if (backoffMs_ > WIFI_BACKOFF_MAX_MS) {
      backoffMs_ = WIFI_BACKOFF_MAX_MS;
    }
  }
}

bool WifiManager::isConnected() const {
  return WiFi.status() == WL_CONNECTED;
}

int8_t WifiManager::rssi() const {
  if (WiFi.status() != WL_CONNECTED) {
    return 0;
  }
  return static_cast<int8_t>(WiFi.RSSI());
}
