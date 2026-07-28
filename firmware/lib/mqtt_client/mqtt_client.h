#pragma once

// MQTT publish/subscribe against the project's contract
// (.claude/skills/mqtt-contract/SKILL.md). Only ever compiled for the
// esp32dev target, same reasoning as wifi_manager: nothing under test/
// includes this.

#include <PubSubClient.h>
#include <WiFi.h>

#include <cstdint>

using TriggerCommandHandler = void (*)(const char* requestId, const char* source, unsigned long ts);

class MqttClient {
public:
  void begin(const char* host, uint16_t port, const char* clientId, const char* username,
             const char* password);
  void setTriggerHandler(TriggerCommandHandler handler);

  // Call every loop iteration. No-ops entirely if !wifiConnected - the one
  // cross-module state check main.cpp's loop passes in, since it's
  // WifiManager's own already-computed state, not new logic here.
  void tick(uint32_t nowMs, bool wifiConnected);

  bool isConnected() const;

  // gate/state: retained, QoS 1. ts/uptimeS are unix-seconds and uptime
  // respectively - a different clock domain from nowMs above, so plain
  // unsigned long (not uint32_t) is fine here; the millis()-rollover
  // concern only applies to the local tick/backoff timers.
  void publishState(const char* state, int rssiVal, unsigned long uptimeS, unsigned long ts);
  // gate/cmd/ack: not retained, QoS 1.
  void publishAck(const char* requestId, const char* result, unsigned long ts);
  // gate/availability "online" is published here on connect; "offline" is
  // the LWT, fired by the broker itself on disconnect - never published
  // directly by us.

private:
  WiFiClient wifiClient_;
  mutable PubSubClient client_{wifiClient_}; // mutable: PubSubClient's connected() isn't const

  const char* host_ = nullptr;
  uint16_t port_ = 1883;
  const char* clientId_ = nullptr;
  const char* username_ = nullptr;
  const char* password_ = nullptr;

  uint32_t lastReconnectMs_ = 0;
  uint32_t backoffMs_ = 0;

  TriggerCommandHandler triggerHandler_ = nullptr;

  static MqttClient* instance_;
  static void staticCallback(char* topic, uint8_t* payload, unsigned int len);
  void handleMessage(const char* topic, const uint8_t* payload, unsigned int len);
};
