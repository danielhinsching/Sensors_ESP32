#include <Arduino.h>
#include <WiFi.h>
#include <WiFiClientSecure.h>
#include <HTTPClient.h>
#include <WiFiManager.h>
#include <DHT.h>

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

// ==================== TEMPO ====================
unsigned long lastTelemetry = 0;
const unsigned long telemetryInterval = 5000; // 5 segundos

// Protótipos
void sendTelemetry();

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

  // Telemetria inicial
  sendTelemetry();
  lastTelemetry = millis();
}

void loop() {
  unsigned long nowMillis = millis();

  // Telemetria periódica
  if (nowMillis - lastTelemetry >= telemetryInterval) {
    lastTelemetry = nowMillis;
    sendTelemetry();
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

  // Serial
  Serial.println("Leituras:");
  if (dht1_ok) Serial.printf("DHT Interno: %.2f C | %.2f %%\n", t1, h1);
  if (dht2_ok) Serial.printf("DHT Externo: %.2f C | %.2f %%\n", t2, h2);
  Serial.printf("Gás - Analog: %d (%.2f V) | Digital: %d | Alarme: %s\n\n",
                gasAnalog, gasVoltage, gasDigital, alarm ? "SIM" : "NÃO");

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

  // Envio HTTPS
  if (WiFi.status() == WL_CONNECTED) {
    WiFiClientSecure client;
    client.setInsecure();
    HTTPClient https;
    https.setTimeout(5000); // timeout 5s

    String url = String("https://mytb.fabricadesoftware.ifc.edu.br/api/v1/") + token + "/telemetry";
    if (https.begin(client, url)) {
      https.addHeader("Content-Type", "application/json");
      int httpResponseCode = https.POST(payload);
      Serial.print("HTTP Response: ");
      Serial.println(httpResponseCode);
      https.end();
    } else {
      Serial.println("Falha ao iniciar HTTPS.");
    }
  } else {
    Serial.println("Wi-Fi desconectado. Não enviando dados.");
  }
}
