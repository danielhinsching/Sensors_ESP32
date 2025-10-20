#include <Arduino.h>
#include <WiFi.h>
#include <WiFiClientSecure.h>
#include <HTTPClient.h>
#include <WiFiManager.h>
#include <DHT.h>
#include "time.h"

// ==================== CONFIGURAÇÕES ====================
#define DHTPIN1 5
#define DHTPIN2 4
#define DHTTYPE DHT11

DHT dht1(DHTPIN1, DHTTYPE);
DHT dht2(DHTPIN2, DHTTYPE);

const int GAS_ANALOG_PIN  = 33;
const int GAS_DIGITAL_PIN = 14;

const int GREEN_LED = 32;
const int RED_LED   = 13;

const char* token = "3xrlzmli5opwkkxuon1m";
const int ANALOG_THRESHOLD = 200;

// Protótipos
void sendTelemetry();
void setupTime();

// ==================== SETUP ====================
void setup() {
  Serial.begin(115200);
  delay(100);

  // Pinos
  pinMode(GAS_ANALOG_PIN, INPUT);
  pinMode(GAS_DIGITAL_PIN, INPUT);
  pinMode(GREEN_LED, OUTPUT);
  pinMode(RED_LED, OUTPUT);

  // Wi-Fi
  WiFi.mode(WIFI_STA);
  WiFiManager wm;
  wm.setTimeout(180);
  if (!wm.autoConnect("ESP32_ConfigAP")) {
    Serial.println("Falha na conexão WiFi. Reiniciando...");
    delay(3000);
    ESP.restart();
  }
  Serial.print("Wi-Fi conectado. IP: ");
  Serial.println(WiFi.localIP());

  // DHT
  dht1.begin();
  dht2.begin();

  // NTP
  setupTime();

  // Telemetria inicial
  sendTelemetry();
}

// ==================== LOOP ====================
void loop() {
  time_t now = time(nullptr);
  struct tm timeinfo;
  if (!localtime_r(&now, &timeinfo)) return;

  // Verifica se estamos no minuto múltiplo de 15 e segundo 0
  if (timeinfo.tm_sec == 0 && (timeinfo.tm_min % 15 == 0)) {
    sendTelemetry();

    // Espera 1 segundo para não disparar múltiplas vezes
    delay(1000);
  }
}

// ==================== TELEMETRIA ====================
void sendTelemetry() {
  // Leitura DHT
  float t1 = dht1.readTemperature();
  float h1 = dht1.readHumidity();
  bool dht1_ok = !(isnan(t1) || isnan(h1));

  float t2 = dht2.readTemperature();
  float h2 = dht2.readHumidity();
  bool dht2_ok = !(isnan(t2) || isnan(h2));

  // Leitura MQ-2
  int gasAnalog = analogRead(GAS_ANALOG_PIN);
  float gasVoltage = gasAnalog * (5.0f / 4095.0f);
  int gasDigital = digitalRead(GAS_DIGITAL_PIN);
  bool alarm = (gasAnalog >= ANALOG_THRESHOLD) || (gasDigital == HIGH);

  // LEDs
  digitalWrite(GREEN_LED, !alarm);
  digitalWrite(RED_LED, alarm);

  // Hora atual
  time_t now = time(nullptr);
  struct tm timeinfo;
  localtime_r(&now, &timeinfo);
  Serial.printf("Hora atual: %02d:%02d:%02d\n",
                timeinfo.tm_hour, timeinfo.tm_min, timeinfo.tm_sec);

  // JSON
  String payload = "{";
  if (dht1_ok) payload += "\"tempInterna\":" + String(t1, 2) + ",";
  if (dht1_ok) payload += "\"umidInterna\":" + String(h1, 2) + ",";
  if (dht2_ok) payload += "\"tempExterna\":" + String(t2, 2) + ",";
  if (dht2_ok) payload += "\"umidExterna\":" + String(h2, 2) + ",";
  payload += "\"gasAnalog\":" + String(gasAnalog) + ",";
  payload += "\"gasVoltage\":" + String(gasVoltage, 2) + ",";
  payload += "\"gasDigital\":" + String(gasDigital) + ",";
  payload += "\"alarmeGas\":" + String(alarm ? 1 : 0) + "}";

  // Envio
  if (WiFi.status() == WL_CONNECTED) {
    WiFiClientSecure client;
    client.setInsecure();
    HTTPClient https;

    String url = String("https://mytb.fabricadesoftware.ifc.edu.br/api/v1/") + token + "/telemetry";
    https.begin(client, url);
    https.addHeader("Content-Type", "application/json");
    int httpResponseCode = https.POST(payload);
    Serial.print("HTTP Response: ");
    Serial.println(httpResponseCode);
    https.end();
  } else {
    Serial.println("Wi-Fi desconectado. Não enviando dados.");
  }

  // Serial
  Serial.println("Leituras:");
  if (dht1_ok) Serial.printf("DHT Interno: %.2f C | %.2f %%\n", t1, h1);
  if (dht2_ok) Serial.printf("DHT Externo: %.2f C | %.2f %%\n", t2, h2);
  Serial.printf("Gás - Analog: %d (%.2f V) | Digital: %d | Alarme: %s\n\n",
                gasAnalog, gasVoltage, gasDigital, alarm ? "SIM" : "NÃO");
}

// ==================== NTP ====================
void setupTime() {
  const char* ntpServer = "ntp.intranet.araquari.ifc.edu.br";
  configTime(-3 * 3600, 0, ntpServer); // GMT-3 (América/São_Paulo)
  Serial.println("Sincronizando hora via NTP...");

  time_t now = time(nullptr);
  int retries = 0;
  while (now < 24 * 3600 && retries < 10) {
    delay(1000);
    Serial.print(".");
    now = time(nullptr);
    retries++;
  }

  if (now < 24 * 3600) {
    Serial.println("\nFalha ao sincronizar NTP. Continuando sem hora.");
  } else {
    Serial.println("\nHora sincronizada via NTP (América/São_Paulo).");
  }
}
