#include <Arduino.h>
#include <DHT.h>

#define DHTPIN D2       // Pino conectado ao DATA do DHT11 (GPIO4)
#define DHTTYPE DHT11   // Tipo do sensor: DHT11

DHT dht(DHTPIN, DHTTYPE);

void setup() {
  Serial.begin(115200);
  dht.begin();
  Serial.println("Teste de DHT11 no NodeMCU");
}


void loop() {
  delay(2000); // Espera entre leituras (2 segundos)

  float h = dht.readHumidity();
  float t = dht.readTemperature();

  if (isnan(h) || isnan(t)) {
    Serial.println("Falha na leitura do sensor DHT11!");
    return;
  }

  Serial.print("Umidade: ");
  Serial.print(h);
  Serial.print(" %\t");

  Serial.print("Temperatura: ");
  Serial.print(t);
  Serial.println(" °C");
}
