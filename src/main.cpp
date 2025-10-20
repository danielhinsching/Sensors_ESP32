#include <Arduino.h>
#include <WiFi.h>
#include <WiFiClientSecure.h>
#include <HTTPClient.h>
#include <WiFiManager.h>
#include <DHT.h>

void sendTelemetry(); // protótipo da função


// ==================== CONFIGURAÇÕES ====================
// DHT11
#define DHTPIN1 5
#define DHTPIN2 4
#define DHTTYPE DHT11

DHT dht1(DHTPIN1, DHTTYPE);
DHT dht2(DHTPIN2, DHTTYPE);

// Sensor de gás MQ-2
const int GAS_ANALOG_PIN  = 33;
const int GAS_DIGITAL_PIN = 14;

// LEDs
const int GREEN_LED = 32;
const int RED_LED   = 13;

// ThingsBoard
const char* token = "3xrlzmli5opwkkxuon1m";

// Configurações
const int ANALOG_THRESHOLD = 200;
const unsigned long INTERVAL = 15UL * 60UL * 1000UL; // 15 minutos
unsigned long lastMsg = 0;

// ==================== SETUP ====================
void setup() {
  Serial.begin(115200);
  delay(100);

  // Configuração de pinos
  pinMode(GAS_ANALOG_PIN, INPUT);
  pinMode(GAS_DIGITAL_PIN, INPUT);
  pinMode(GREEN_LED, OUTPUT);
  pinMode(RED_LED, OUTPUT);

  // Conexão Wi-Fi
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

  // Inicializa DHTs
  dht1.begin();
  dht2.begin();

  // Telemetria inicial imediata
  sendTelemetry();
  lastMsg = millis();
}

// ==================== LOOP ====================
void loop() {
  if (millis() - lastMsg >= INTERVAL) {
    lastMsg = millis();
    sendTelemetry();
  }
}

// ==================== FUNÇÃO DE TELEMETRIA ====================
void sendTelemetry() {
  // Leitura DHT1
  float t1 = dht1.readTemperature();
  float h1 = dht1.readHumidity();
  bool dht1_ok = !(isnan(t1) || isnan(h1));

  // Leitura DHT2
  float t2 = dht2.readTemperature();
  float h2 = dht2.readHumidity();
  bool dht2_ok = !(isnan(t2) || isnan(h2));

  // Leitura MQ-2
  int gasAnalog = analogRead(GAS_ANALOG_PIN);
  float gasVoltage = gasAnalog * (5.0f / 4095.0f);
  int gasDigital = digitalRead(GAS_DIGITAL_PIN);
  bool alarm = (gasAnalog >= ANALOG_THRESHOLD) || (gasDigital == HIGH);

  // Atualiza LEDs
  digitalWrite(GREEN_LED, !alarm);
  digitalWrite(RED_LED, alarm);

  // Monta JSON
  String payload = "{";
  if (dht1_ok) {
    payload += "\"tempInterna\":" + String(t1, 2) + ",";
    payload += "\"umidInterna\":" + String(h1, 2) + ",";
  } else {
    Serial.println("Falha ao ler DHT interno.");
  }

  if (dht2_ok) {
    payload += "\"tempExterna\":" + String(t2, 2) + ",";
    payload += "\"umidExterna\":" + String(h2, 2) + ",";
  } else {
    Serial.println("Falha ao ler DHT externo.");
  }

  payload += "\"gasAnalog\":" + String(gasAnalog) + ",";
  payload += "\"gasVoltage\":" + String(gasVoltage, 2) + ",";
  payload += "\"gasDigital\":" + String(gasDigital) + ",";
  payload += "\"alarmeGas\":" + String(alarm ? 1 : 0) + "}";

  // Envio para ThingsBoard
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

  // Impressão serial limpa
  Serial.println("Leituras:");
  if (dht1_ok) Serial.printf("DHT Interno: %.2f C | %.2f %%\n", t1, h1);
  if (dht2_ok) Serial.printf("DHT Externo: %.2f C | %.2f %%\n", t2, h2);
  Serial.printf("Gás - Analog: %d (%.2f V) | Digital: %d | Alarme: %s\n\n",
                gasAnalog, gasVoltage, gasDigital, alarm ? "SIM" : "NÃO");
}
