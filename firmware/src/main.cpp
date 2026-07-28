#include <Arduino.h>
#include <ArduinoOTA.h>
#include <esp_task_wdt.h>
#include <time.h>

#include "config.h"
#include "gate_control.h"
#include "logging.h"
#include "mqtt_client.h"
#include "secrets.h" // developer-provided, gitignored - copy from secrets.h.example
#include "status_decoder.h"
#include "wifi_manager.h"

// main.cpp stays thin: wire modules together, dispatch tick()s. No business
// logic lives here (.claude/skills/esp32-firmware/SKILL.md).

GateControl gateControl;
StatusDecoder statusDecoder;
WifiManager wifiManager;
MqttClient mqttClient;

// uint32_t (not unsigned long) for the millis()-domain clock: matches the
// real 32-bit width of millis() on the target and the rollover-safe
// arithmetic used throughout gate_control/status_decoder/wifi_manager.
uint32_t bootMs = 0;
uint32_t lastHeartbeatMs = 0;

unsigned long nowUnixSeconds() {
  time_t now;
  time(&now);
  return static_cast<unsigned long>(now);
}

void onTriggerCommand(const char* requestId, const char* source, unsigned long ts) {
  GateControlResult r = gateControl.requestTrigger(requestId, millis());
  mqttClient.publishAck(requestId, gateControlResultToString(r), nowUnixSeconds());
  LOG_INFO("main: trigger request_id=%s source=%s result=%s", requestId, source,
           gateControlResultToString(r));
  (void)ts; // command timestamp is logged upstream in mqtt_client; not otherwise used yet
}

void setup() {
  // Boot safety: relay forced to its idle level before anything else runs,
  // and before pinMode(OUTPUT) is ever called on it. Must stay the first
  // line of setup().
  gateControlHardwareInit();

  Serial.begin(115200);
  pinMode(STATUS_SENSE_PIN, INPUT); // no pull-up: the resistor divider drives a real voltage

  uint32_t now = millis();
  bootMs = now;

  StatusDecoderConfig statusCfg = {
      STATUS_SAMPLE_INTERVAL_MS,       STATUS_DEBOUNCE_MS,
      STATUS_STEADY_CONFIRM_MS,        STATUS_SLOW_FLASH_MIN_MS,
      STATUS_SLOW_FLASH_MAX_MS,        STATUS_FAST_FLASH_MIN_MS,
      STATUS_FAST_FLASH_MAX_MS,        STATUS_FAULT_FLASH_TOLERANCE_MS,
      STATUS_SENSE_ACTIVE_HIGH_PLACEHOLDER,
  };
  statusDecoder.begin(statusCfg, now);
  gateControl.begin(relayPinWrite, now);

  esp_task_wdt_init(WDT_TIMEOUT_S, true);
  esp_task_wdt_add(NULL);

  wifiManager.begin(WIFI_SSID, WIFI_PASSWORD);

  mqttClient.begin(MQTT_BROKER_HOST, MQTT_BROKER_PORT, MQTT_CLIENT_ID, MQTT_USERNAME,
                    MQTT_PASSWORD);
  mqttClient.setTriggerHandler(onTriggerCommand);

  configTime(0, 0, "pool.ntp.org"); // best-effort; ts may read ~0 until NTP syncs

  ArduinoOTA.setHostname(OTA_HOSTNAME);
  ArduinoOTA.setPassword(OTA_PASSWORD);
  ArduinoOTA.begin();

  lastHeartbeatMs = now;
}

void loop() {
  esp_task_wdt_reset();
  ArduinoOTA.handle();

  uint32_t now = millis();

  wifiManager.tick(now);
  mqttClient.tick(now, wifiManager.isConnected());

  bool raw = statusSenseReadRaw();
  bool changed = statusDecoder.tick(raw, now);
  gateControl.tick(now);

  if (changed || (now - lastHeartbeatMs) >= STATE_HEARTBEAT_INTERVAL_MS) {
    mqttClient.publishState(gateStateToString(statusDecoder.currentState()), wifiManager.rssi(),
                             (now - bootMs) / 1000, nowUnixSeconds());
    lastHeartbeatMs = now;
  }
}
