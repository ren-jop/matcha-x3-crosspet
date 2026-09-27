#include "AnkiMineQueue.h"

#include <Arduino.h>
#include <ArduinoJson.h>
#include <HTTPClient.h>
#include <HalStorage.h>
#include <Logging.h>
#include <WiFi.h>

#include <atomic>
#include <cstring>

namespace {
constexpr char kDirectory[] = "/AnkiOutbox";
constexpr char kConfigPath[] = "/anki-wifi.json";
constexpr size_t kMaxPayload = 4096;
std::atomic<bool> pumpRunning{false};

bool validText(const char* text, size_t limit) { return text && strnlen(text, limit + 1) <= limit; }

void pumpTask(void*) {
  AnkiMineQueue::pump();
  pumpRunning.store(false, std::memory_order_release);
  vTaskDelete(nullptr);
}
}  // namespace

void AnkiMineQueue::pumpAsync() {
  if (WiFi.status() != WL_CONNECTED || pumpRunning.exchange(true, std::memory_order_acq_rel)) return;
  if (xTaskCreate(pumpTask, "AnkiMine", 6144, nullptr, 1, nullptr) != pdPASS) {
    pumpRunning.store(false, std::memory_order_release);
    LOG_ERR("ANKI", "Unable to start Wi-Fi mine sender");
  }
}

bool AnkiMineQueue::enqueue(const char* expression, const char* reading, const char* meaning) {
  if (!validText(expression, 240) || !validText(reading, 240) || !validText(meaning, 3000) || !*expression)
    return false;
  if (!Storage.ensureDirectoryExists(kDirectory)) return false;

  char id[40];
  snprintf(id, sizeof(id), "x3-%08lx-%08lx", static_cast<unsigned long>(millis()),
           static_cast<unsigned long>(esp_random()));
  JsonDocument doc;
  doc["id"] = id;
  doc["fields"]["expression"] = expression;
  doc["fields"]["reading"] = reading;
  doc["fields"]["meaning"] = meaning;
  String payload;
  serializeJson(doc, payload);
  if (payload.length() > kMaxPayload) return false;
  const String path = String(kDirectory) + "/" + id + ".json";
  return Storage.writeFile(path.c_str(), payload);
}

void AnkiMineQueue::pump() {
  if (WiFi.status() != WL_CONNECTED || !Storage.ready() || !Storage.exists(kConfigPath)) return;
  // Bound the work to one note per attempt. Failed requests
  // leave the same file and ID in place for an idempotent retry.
  const auto files = Storage.listFiles(kDirectory, 1);
  if (files.empty()) return;
  String path = files.front();
  if (!path.startsWith("/")) path = String(kDirectory) + "/" + path;
  if (!path.endsWith(".json")) return;
  const String payload = Storage.readFile(path.c_str());
  if (payload.isEmpty() || payload.length() > kMaxPayload) return;

  const String configText = Storage.readFile(kConfigPath);
  if (configText.length() > 1024) return;
  JsonDocument config;
  if (deserializeJson(config, configText)) return;
  const char* url = config["url"] | "";
  const char* token = config["token"] | "";
  if (strncmp(url, "http://", 7) != 0 || strlen(url) > 180 || strlen(token) < 24 || strlen(token) > 128) return;

  WiFiClient client;
  HTTPClient http;
  if (!http.begin(client, url)) return;
  http.setTimeout(3000);
  http.addHeader("Content-Type", "application/json");
  http.addHeader("Authorization", String("Bearer ") + token);
  const int code = http.POST(payload);
  http.end();
  if (code == 202) {
    Storage.remove(path.c_str());
    LOG_INF("ANKI", "Mine accepted by desktop inbox");
  } else {
    LOG_DBG("ANKI", "Desktop inbox unavailable (%d)", code);
  }
}
