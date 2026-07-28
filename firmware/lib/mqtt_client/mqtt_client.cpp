#include "mqtt_client.h"

#include <ArduinoJson.h>

#include <cstring>

#include "config.h"
#include "logging.h"

MqttClient* MqttClient::instance_ = nullptr;

void MqttClient::begin(const char* host, uint16_t port, const char* clientId,
                        const char* username, const char* password) {
  host_ = host;
  port_ = port;
  clientId_ = clientId;
  username_ = username;
  password_ = password;
  backoffMs_ = MQTT_BACKOFF_INITIAL_MS;
  lastReconnectMs_ = 0;

  instance_ = this;
  client_.setServer(host_, port_);
  client_.setCallback(staticCallback);
  client_.setKeepAlive(MQTT_KEEPALIVE_S);
}

void MqttClient::setTriggerHandler(TriggerCommandHandler handler) {
  triggerHandler_ = handler;
}

void MqttClient::tick(uint32_t nowMs, bool wifiConnected) {
  if (!wifiConnected) {
    return;
  }

  if (client_.connected()) {
    client_.loop();
    backoffMs_ = MQTT_BACKOFF_INITIAL_MS;
    return;
  }

  if ((nowMs - lastReconnectMs_) >= backoffMs_) {
    lastReconnectMs_ = nowMs;
    LOG_INFO("mqtt: connecting to %s:%u", host_, port_);

    bool ok = client_.connect(clientId_, username_, password_, TOPIC_AVAILABILITY, 1, true,
                               "offline");
    if (ok) {
      LOG_INFO("mqtt: connected");
      client_.publish(TOPIC_AVAILABILITY, "online", true);
      client_.subscribe(TOPIC_CMD_TRIGGER, 1);
      backoffMs_ = MQTT_BACKOFF_INITIAL_MS;
    } else {
      uint32_t jitter = random(0, backoffMs_ / 4 + 1);
      uint32_t doubled = backoffMs_ * 2;
      if (doubled > MQTT_BACKOFF_MAX_MS) {
        doubled = MQTT_BACKOFF_MAX_MS;
      }
      backoffMs_ = doubled + jitter;
      if (backoffMs_ > MQTT_BACKOFF_MAX_MS) {
        backoffMs_ = MQTT_BACKOFF_MAX_MS;
      }
      LOG_WARN("mqtt: connect failed, rc=%d, retrying in %lums", client_.state(),
                (unsigned long)backoffMs_);
    }
  }
}

bool MqttClient::isConnected() const {
  return client_.connected();
}

void MqttClient::publishState(const char* state, int rssiVal, unsigned long uptimeS,
                                unsigned long ts) {
  JsonDocument doc;
  doc["v"] = SCHEMA_VERSION;
  doc["state"] = state;
  doc["rssi"] = rssiVal;
  doc["uptime_s"] = uptimeS;
  doc["ts"] = ts;

  char buf[256];
  size_t n = serializeJson(doc, buf, sizeof(buf));
  client_.publish(TOPIC_STATE, reinterpret_cast<const uint8_t*>(buf), n, true); // retained
}

void MqttClient::publishAck(const char* requestId, const char* result, unsigned long ts) {
  JsonDocument doc;
  doc["v"] = SCHEMA_VERSION;
  doc["request_id"] = requestId;
  doc["result"] = result;
  doc["ts"] = ts;

  char buf[192];
  size_t n = serializeJson(doc, buf, sizeof(buf));
  client_.publish(TOPIC_CMD_ACK, reinterpret_cast<const uint8_t*>(buf), n, false); // not retained
}

void MqttClient::staticCallback(char* topic, uint8_t* payload, unsigned int len) {
  if (instance_ != nullptr) {
    instance_->handleMessage(topic, payload, len);
  }
}

void MqttClient::handleMessage(const char* topic, const uint8_t* payload, unsigned int len) {
  if (strcmp(topic, TOPIC_CMD_TRIGGER) != 0) {
    return;
  }

  JsonDocument doc;
  DeserializationError err = deserializeJson(doc, payload, len);
  if (err) {
    LOG_WARN("mqtt: failed to parse trigger payload: %s", err.c_str());
    return;
  }

  if (!doc["v"].is<int>() || doc["v"].as<int>() != SCHEMA_VERSION) {
    LOG_WARN("mqtt: trigger payload has unknown/missing schema version");
    return;
  }

  const char* requestId = doc["request_id"].as<const char*>();
  if (requestId == nullptr || strnlen(requestId, REQUEST_ID_MAX_LEN) >= REQUEST_ID_MAX_LEN) {
    LOG_WARN("mqtt: trigger payload has missing/oversized request_id");
    return;
  }

  const char* source = doc["source"].as<const char*>();
  if (source == nullptr) {
    source = "unknown";
  }
  unsigned long ts = doc["ts"].as<unsigned long>();

  if (triggerHandler_ != nullptr) {
    triggerHandler_(requestId, source, ts);
  }
}
