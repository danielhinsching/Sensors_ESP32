#include <WiFi.h>
#include <WiFiClientSecure.h>
#include <HTTPClient.h>
#include <WiFiManager.h>
#include <DHT.h>

// Configuração dos sensores DHT22
#define DHTPIN_INTERNO 18
#define DHTPIN_EXTERNO 26
#define DHTTYPE DHT22

DHT dhtInterno(DHTPIN_INTERNO, DHTTYPE);
DHT dhtExterno(DHTPIN_EXTERNO, DHTTYPE);

// Pinos dos sensores adicionais
const int gasAnalogPin = 39;
const int gasDigitalPin = 4;
const int redLed = 32;
const int greenLed = 26;
const int relePin = 23;

const char* token = "Jf8Wa1TFYInsq9q9elvo";

unsigned long lastMsg = 0;
const long interval = 2000; // 2 segundos

void setup() {
  Serial.begin(115200);

  // Configurações de pinos
  pinMode(gasAnalogPin, INPUT);
  pinMode(gasDigitalPin, INPUT);
  pinMode(redLed, OUTPUT);
  pinMode(greenLed, OUTPUT);
  pinMode(relePin, OUTPUT);

  // Conexão Wi-Fi
  WiFi.mode(WIFI_STA);
  delay(1000);

  WiFiManager wm;
  wm.setTimeout(180);

  if (!wm.autoConnect("ESP32_ConfigAP")) {
    Serial.println("⚠️ Falha na conexão ou timeout expirado. Reiniciando...");
    delay(3000);
    ESP.restart();
  }

  Serial.println("✅ Wi-Fi conectado!");
  Serial.println(WiFi.localIP());

  // Inicialização dos sensores
  dhtInterno.begin();
  dhtExterno.begin();

  // Teste inicial
  Serial.println("Teste dos sensores DHT22:");
  float tInt = dhtInterno.readTemperature();
  float hInt = dhtInterno.readHumidity();
  float tExt = dhtExterno.readTemperature();
  float hExt = dhtExterno.readHumidity();

  if (isnan(tInt) || isnan(hInt) || isnan(tExt) || isnan(hExt)) {
    Serial.println("❌ Erro ao ler um ou mais sensores DHT22");
  } else {
    Serial.println("✅ Sensores funcionando:");
    Serial.printf("Interno -> Temp: %.2f°C | Umid: %.2f%%\n", tInt, hInt);
    Serial.printf("Externo -> Temp: %.2f°C | Umid: %.2f%%\n", tExt, hExt);
  }
}

void loop() {
  unsigned long now = millis();
  if (now - lastMsg > interval) {
    lastMsg = now;

    float tInt = dhtInterno.readTemperature();
    float hInt = dhtInterno.readHumidity();
    float tExt = dhtExterno.readTemperature();
    float hExt = dhtExterno.readHumidity();
    int gasValue = analogRead(gasAnalogPin);

    if (isnan(tInt) || isnan(hInt) || isnan(tExt) || isnan(hExt)) {
      Serial.println("❌ Erro ao ler os sensores DHT22");
      return;
    }

    // Payload com dados dos dois sensores
    String payload = "{";
    payload += "\"tempInterna\":" + String(tInt, 2) + ",";
    payload += "\"umidInterna\":" + String(hInt, 2) + ",";
    payload += "\"tempExterna\":" + String(tExt, 2) + ",";
    payload += "\"umidExterna\":" + String(hExt, 2) + ",";
    payload += "\"gas\":" + String(gasValue);
    payload += "}";

    Serial.println("Publicando via HTTPS:");
    Serial.println(payload);

    if (WiFi.status() == WL_CONNECTED) {
      WiFiClientSecure client;
      client.setInsecure();

      HTTPClient https;
      https.begin(client, String("https://mytb.fabricadesoftware.ifc.edu.br/api/v1/") + token + "/telemetry");
      https.addHeader("Content-Type", "application/json");

      int httpResponseCode = https.POST(payload);

      Serial.printf("🌡️ Interna: %.2f°C | 💧 %.2f%%\n", tInt, hInt);
      Serial.printf("🌡️ Externa: %.2f°C | 💧 %.2f%%\n", tExt, hExt);
      Serial.print("📡 Resposta HTTP: ");
      Serial.println(httpResponseCode);

      https.end();
    } else {
      Serial.println("❌ Wi-Fi desconectado.");
    }
  }
}
