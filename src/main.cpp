#include <Arduino.h>
#include <WiFi.h>
#include <PubSubClient.h>
#include <DHT.h>
#include <WiFiManager.h>

#define DHTTYPE DHT11
#define DHTPIN 33
DHT dht(DHTPIN, DHTTYPE);

const int gasAnalogPin = 39;
const int gasDigitalPin = 4;
const int redLed = 32;
const int greenLed = 26;
const int relePin = 23;

WiFiClient espClient;
PubSubClient client(espClient);

unsigned long lastMsg = 0;
const long interval = 900000;  // 15 minutos

const char* mqtt_server = "mytb.fabricadesoftware.ifc.edu.br";
const int mqtt_port = 1883;
const char* token = "token do dispositivo criado";   // Substitua pelo token do dispositivo no ThingsBoard

void setup_wifi() {
  WiFiManager wifiManager;
  wifiManager.autoConnect("AutoConnectAP");
  Serial.println("WiFi conectado");
}

void reconnect() {
  while (!client.connected()) {
    Serial.print("Tentando conectar ao MQTT...");
    if (client.connect("ESP32Client", token, NULL)) {
      Serial.println("conectado");
    } else {
      Serial.print("falhou, rc=");
      Serial.print(client.state());
      Serial.println(" tentando novamente em 5 segundos");
      delay(5000);
    }
  }
}

void setup() {
  Serial.begin(115200);
  pinMode(redLed, OUTPUT);
  pinMode(greenLed, OUTPUT);
  pinMode(relePin, OUTPUT);
  dht.begin();

  setup_wifi();
  client.setServer(mqtt_server, mqtt_port);
}

void loop() {
  if (!client.connected()) {
    reconnect();
  }
  client.loop();

  unsigned long now = millis();
  if (now - lastMsg > interval) {
    lastMsg = now;

    float temp = dht.readTemperature();
    float hum = dht.readHumidity();
    int gasValue = analogRead(gasAnalogPin);

    // Formata os dados em JSON
    String payload = "{";
    payload += "\"temperatura\":" + String(temp) + ",";
    payload += "\"umidade\":" + String(hum) + ",";
    payload += "\"gas\":" + String(gasValue);
    payload += "}";

    Serial.println("Publicando: ");
    Serial.println(payload);

    client.publish("v1/devices/me/telemetry", payload.c_str());
  }
}
