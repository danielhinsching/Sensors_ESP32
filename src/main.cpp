#include <WiFi.h>
#include <WiFiClientSecure.h>
#include <HTTPClient.h>
#include <WiFiManager.h>

#define DHTPIN 33
#define DHTTYPE DHT11
const int gasAnalogPin = 39;
const int gasDigitalPin = 4;
const int redLed = 32;
const int greenLed = 26;
const int relePin = 23;

const char* token = "Jf8Wa1TFYInsq9q9elvo";

unsigned long lastMsg = 0;
const long interval = 1000;

float randomFloat(float minVal, float maxVal) {
  return minVal + ((float)random() / (float)UINT32_MAX) * (maxVal - minVal);
}

int randomInt(int minVal, int maxVal) {
  return minVal + random(maxVal - minVal + 1);
}

void setup() {
  Serial.begin(115200);

  pinMode(gasAnalogPin, INPUT);
  pinMode(gasDigitalPin, INPUT);
  pinMode(redLed, OUTPUT);
  pinMode(greenLed, OUTPUT);
  pinMode(relePin, OUTPUT);

  WiFi.mode(WIFI_STA);
  delay(1000);

  WiFiManager wm;
  wm.setTimeout(180); // Timeout 3 minutos para configuração

  if (!wm.autoConnect("ESP32_ConfigAP")) {
    Serial.println("⚠️ Falha na conexão ou timeout expirado. Reiniciando...");
    delay(3000);
    ESP.restart();
  }

  Serial.println("✅ Wi-Fi conectado!");
  Serial.println(WiFi.localIP());

  randomSeed(analogRead(0)); // Inicializa o gerador de números aleatórios
}

void loop() {
  unsigned long now = millis();
  if (now - lastMsg > interval) {
    lastMsg = now;

    // Gerar valores aleatórios para temperatura, umidade e gás
    float temp = randomFloat(20.0, 30.0);
    float hum = randomFloat(30.0, 70.0);
    int gasValue = randomInt(0, 4095);

    String payload = "{\"temperature\":" + String(temp, 1) +
                     ",\"humidity\":" + String(hum, 1) +
                     ",\"gas\":" + String(gasValue) + "}";

    Serial.println("Publicando via HTTPS:");
    Serial.println(payload);

    if (WiFi.status() == WL_CONNECTED) {
      WiFiClientSecure client;
      client.setInsecure();  // Aceita conexão sem verificar certificado

      HTTPClient https;
      https.begin(client, String("https://mytb.fabricadesoftware.ifc.edu.br/api/v1/") + token + "/telemetry");
      https.addHeader("Content-Type", "application/json");

      int httpResponseCode = https.POST(payload);

      Serial.print("Resposta HTTP: ");
      Serial.println(httpResponseCode);

      https.end();
    } else {
      Serial.println("Wi-Fi desconectado.");
    }
  }
}
