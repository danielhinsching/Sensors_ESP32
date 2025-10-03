#include <Arduino.h>
#include <WiFi.h>
#include <WiFiClientSecure.h>
#include <HTTPClient.h>
#include <WiFiManager.h>
#include <DHT.h>

// ==================== CONFIGURAÇÕES ====================
// DHT11
#define DHTPIN1 5   // RJ2 -> D5
#define DHTPIN2 4   // RJ1 -> D4
#define DHTTYPE DHT11

DHT dht1(DHTPIN1, DHTTYPE);
DHT dht2(DHTPIN2, DHTTYPE);

// Sensor de gás MQx
const int GAS_ANALOG_PIN  = 33; 
const int GAS_DIGITAL_PIN = 14; 

// LEDs
const int GREEN_LED = 32; 
const int RED_LED   = 13; 

// ThingsBoard
const char* token = "Jf8Wa1TFYInsq9q9elvo";  

const int ANALOG_THRESHOLD = 2000; 
const unsigned long INTERVAL = 3000; 
unsigned long lastMsg = 0;

// ==================== SETUP ====================
void setup() {
  Serial.begin(115200);
  delay(100);

  // Configurações de pinos
  pinMode(GAS_ANALOG_PIN, INPUT);
  pinMode(GAS_DIGITAL_PIN, INPUT);
  pinMode(GREEN_LED, OUTPUT);
  pinMode(RED_LED, OUTPUT);

  // WiFi Manager
  WiFi.mode(WIFI_STA);
  WiFiManager wm;
  wm.setTimeout(180);
  if (!wm.autoConnect("ESP32_ConfigAP")) {
    Serial.println("⚠️ Falha na conexão WiFi. Reiniciando...");
    delay(3000);
    ESP.restart();
  }

  Serial.println("✅ Wi-Fi conectado!");
  Serial.println(WiFi.localIP());

  // Inicialização dos DHT11
  dht1.begin();
  dht2.begin();
}

// ==================== LOOP ====================
void loop() {
  unsigned long now = millis();
  if (now - lastMsg > INTERVAL) {
    lastMsg = now;

    // ==================== LEITURA DHT11 ====================
    float t1 = dht1.readTemperature();
    float h1 = dht1.readHumidity();
    float t2 = dht2.readTemperature();
    float h2 = dht2.readHumidity();

    bool dht1_ok = !(isnan(t1) || isnan(h1));
    bool dht2_ok = !(isnan(t2) || isnan(h2));

    if (!dht1_ok) {
      Serial.println("❌ Erro no DHT11 interno (RJ2)");
    }
    if (!dht2_ok) {
      Serial.println("❌ Erro no DHT11 externo (RJ1)");
    }

    // ==================== LEITURA GÁS ====================
    int gasAnalog = analogRead(GAS_ANALOG_PIN);
    float gasVoltage = gasAnalog * (3.3f / 4095.0f);
    int gasDigital = digitalRead(GAS_DIGITAL_PIN);

    // Se não houver sensor, filtra valores inválidos
    bool gas_ok = !(gasAnalog < 50 && gasDigital == 0); // heurística
    bool alarmByAnalog = gas_ok && (gasAnalog >= ANALOG_THRESHOLD);
    bool alarmByDigital = gas_ok && (gasDigital == HIGH);
    bool alarm = alarmByAnalog || alarmByDigital;

    // LEDs de status
    if (alarm) {
      digitalWrite(RED_LED, HIGH);
      digitalWrite(GREEN_LED, LOW);
    } else {
      digitalWrite(RED_LED, LOW);
      digitalWrite(GREEN_LED, HIGH);
    }

    // ==================== SERIAL PRINT ====================
    Serial.println("============ 📊 LEITURA ============");
    if (dht1_ok) {
      Serial.printf("🌡️ DHT Interno: %.2f°C | 💧 %.2f%%\n", t1, h1);
    } else {
      Serial.println("❌ Erro no DHT Interno (RJ2)");
    }

    if (dht2_ok) {
      Serial.printf("🌡️ DHT Externo: %.2f°C | 💧 %.2f%%\n", t2, h2);
    } else {
      Serial.println("❌ Erro no DHT Externo (RJ1)");
    }

    if (gas_ok) {
      Serial.printf("🛑 Gás Analog: %d (~%.2f V) | Digital: %d | Alarme: %s\n",
                    gasAnalog, gasVoltage, gasDigital, alarm ? "SIM 🚨" : "não");
    } else {
      Serial.println("⚠️ Sensor de gás não detectado.");
    }
    Serial.println("====================================");

    // ==================== JSON ====================
    String payload = "{";
    if (dht1_ok) {
      payload += "\"tempInterna\":" + String(t1, 2) + ",";
      payload += "\"umidInterna\":" + String(h1, 2) + ",";
    }
    if (dht2_ok) {
      payload += "\"tempExterna\":" + String(t2, 2) + ",";
      payload += "\"umidExterna\":" + String(h2, 2) + ",";
    }
    if (gas_ok) {
      payload += "\"gasAnalog\":" + String(gasAnalog) + ",";
      payload += "\"gasVoltage\":" + String(gasVoltage, 2) + ",";
      payload += "\"gasDigital\":" + String(gasDigital) + ",";
      payload += "\"alarmeGas\":" + String(alarm ? 1 : 0);
    } else {
      payload += "\"sensorGas\":0"; // indica ausência
    }
    payload += "}";

    // Publicação
    Serial.println("📡 Publicando:");
    Serial.println(payload);

    // ==================== ENVIO HTTPS ====================
    if (WiFi.status() == WL_CONNECTED) {
      WiFiClientSecure client;
      client.setInsecure();
      HTTPClient https;

      https.begin(client, String("https://mytb.fabricadesoftware.ifc.edu.br/api/v1/") + token + "/telemetry");
      https.addHeader("Content-Type", "application/json");

      int httpResponseCode = https.POST(payload);

      if (dht1_ok) {
        Serial.printf("🌡️ Interna: %.2f°C | 💧 %.2f%%\n", t1, h1);
      }
      if (dht2_ok) {
        Serial.printf("🌡️ Externa: %.2f°C | 💧 %.2f%%\n", t2, h2);
      }
      Serial.printf("Gas Analog: %d (~%.2f V) | Digital: %d | Alarme: %s\n",
                    gasAnalog, gasVoltage, gasDigital, alarm ? "SIM 🚨" : "não");
      Serial.print("Resposta HTTP: ");
      Serial.println(httpResponseCode);

      https.end();
    } else {
      Serial.println("❌ Wi-Fi desconectado.");
    }
  }
}
// ==================== FIM ====================  