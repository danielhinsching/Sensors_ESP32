#include <WiFi.h>
#include <WiFiClientSecure.h>
#include <HTTPClient.h>
#include <WiFiManager.h>
#include <DHT.h>

#define DHTPIN 33
#define DHTTYPE DHT11
DHT dht(DHTPIN, DHTTYPE);

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

  pinMode(gasAnalogPin, INPUT);
  pinMode(gasDigitalPin, INPUT);
  pinMode(redLed, OUTPUT);
  pinMode(greenLed, OUTPUT);
  pinMode(relePin, OUTPUT);

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

  dht.begin();

  // Teste inicial do sensor
  Serial.println("Teste do sensor DHT11:");
  float tempTeste = dht.readTemperature();
  float humTeste = dht.readHumidity();
  if (isnan(tempTeste) || isnan(humTeste)) {
    Serial.println("❌ Erro inicial na leitura do sensor DHT11");
  } else {
    Serial.println("✅ Sensor DHT11 OK");
    Serial.print("Temperatura: "); Serial.println(tempTeste);
    Serial.print("Umidade: "); Serial.println(humTeste);
  }
}

void loop() {
  unsigned long now = millis();
  if (now - lastMsg > interval) {
    lastMsg = now;

    float temp = dht.readTemperature();
    float hum = dht.readHumidity();
    int gasValue = analogRead(gasAnalogPin);

    if (isnan(temp) || isnan(hum)) {
      Serial.println("❌ Erro ao ler o sensor DHT11");
      return;
    }

    String payload = "{\"temperature\":" + String(temp, 1) +
                     ",\"humidity\":" + String(hum, 1) +
                    //  ",\"gas\":" + String(gasValue) + "}";

    Serial.println("Publicando via HTTPS:");
    Serial.println(payload);

    if (WiFi.status() == WL_CONNECTED) {
      WiFiClientSecure client;
      client.setInsecure();

      HTTPClient https;
      https.begin(client, String("https://mytb.fabricadesoftware.ifc.edu.br/api/v1/") + token + "/telemetry");
      https.addHeader("Content-Type", "application/json");

      int httpResponseCode = https.POST(payload);
      Serial.print("Temperatura (°C): ");
Serial.println(temp);
Serial.print("Umidade (%): ");
Serial.println(hum);


      Serial.print("Resposta HTTP: ");
      Serial.println(httpResponseCode);

      https.end();
    } else {
      Serial.println("Wi-Fi desconectado.");
    }
  }
}
