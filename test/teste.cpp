#include <WiFi.h>

void setup() {
  Serial.begin(115200);
  delay(1000);
  Serial.println("MAC address da placa:");
  Serial.println(WiFi.macAddress());
}

void loop() {
  // Nada aqui
}
