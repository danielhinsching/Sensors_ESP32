#include <WiFi.h>
#include <WiFiClientSecure.h>
#include <HTTPClient.h>
#include <WiFiManager.h>
#include <DHT.h>

#define DHTPIN_INTERNO 23
#define DHTPIN_EXTERNO 32
#define DHTTYPE DHT22
DHT dhtInterno(DHTPIN_INTERNO, DHTTYPE);
DHT dhtExterno(DHTPIN_EXTERNO, DHTTYPE);

const int redLed = 19;
const int greenLed = 18;

const char* token = "cfXspXMCU6FBa8l9IDqa";

unsigned long lastMsg = 0;
const unsigned long interval = 15 * 60 * 1000; // 15 minutos

// === LED visual ===
void ledsPadraoFuncionamento() {
  digitalWrite(greenLed, HIGH);
  digitalWrite(redLed, LOW);
}
void ledsLendoSensores() {
  digitalWrite(greenLed, HIGH);
  digitalWrite(redLed, HIGH);
  delay(300);
  digitalWrite(redLed, LOW);
  delay(300);
}
void ledsEnviandoDados() {
  for (int i = 0; i < 3; i++) {
    digitalWrite(greenLed, LOW);
    digitalWrite(redLed, HIGH);
    delay(300);
    digitalWrite(redLed, LOW);
    digitalWrite(greenLed, HIGH);
    delay(300);
  }
}
void ledsSomenteUmSensor() {
  digitalWrite(greenLed, LOW);
  for (int i = 0; i < 2; i++) {
    digitalWrite(redLed, HIGH);
    delay(300);
    digitalWrite(redLed, LOW);
    delay(300);
  }
}
void ledsSemSensores() {
  digitalWrite(greenLed, LOW);
  digitalWrite(redLed, HIGH);
}
void piscarLedsSincronizados(int vezes = 3, int intervalo = 300) {
  for (int i = 0; i < vezes; i++) {
    digitalWrite(greenLed, HIGH);
    digitalWrite(redLed, HIGH);
    delay(intervalo);
    digitalWrite(greenLed, LOW);
    digitalWrite(redLed, LOW);
    delay(intervalo);
  }
}

// === Inicialização dos sensores com tolerância ===
void inicializarSensoresComTolerancia() {
  dhtInterno.begin();
  dhtExterno.begin();
  bool leituraOk = false;

  for (int i = 0; i < 5; i++) {
    float tInt = dhtInterno.readTemperature();
    float hInt = dhtInterno.readHumidity();
    float tExt = dhtExterno.readTemperature();
    float hExt = dhtExterno.readHumidity();

    if (!isnan(tInt) || !isnan(tExt)) {
      leituraOk = true;
      break;
    }

    Serial.println("⏳ Aguardando sensores...");
    delay(2000); // Espera antes de tentar de novo
  }

  if (leituraOk) {
    Serial.println("✅ Sensores prontos!");
    ledsPadraoFuncionamento();
  } else {
    Serial.println("⚠️ Sensores não responderam inicialmente. Continuando mesmo assim.");
    ledsSomenteUmSensor(); // Indica estado de incerteza
  }
}

// === Enviar leitura atual ===
void enviarLeitura() {
  ledsLendoSensores();

  float tInt = dhtInterno.readTemperature();
  float hInt = dhtInterno.readHumidity();
  float tExt = dhtExterno.readTemperature();
  float hExt = dhtExterno.readHumidity();

  bool internoValido = !(isnan(tInt) || isnan(hInt));
  bool externoValido = !(isnan(tExt) || isnan(hExt));

  if (!internoValido && !externoValido) {
    Serial.println("❌ Nenhum sensor disponível.");
    ledsSemSensores();
    return;
  }

  if (internoValido && externoValido) {
    ledsPadraoFuncionamento();
  } else {
    ledsSomenteUmSensor();
  }

  String payload = "{";
  bool adicionou = false;

  if (internoValido) {
    payload += "\"tempInterna\":" + String(tInt, 2);
    payload += ",\"umidInterna\":" + String(hInt, 2);
    adicionou = true;
  }
  if (externoValido) {
    if (adicionou) payload += ",";
    payload += "\"tempExterna\":" + String(tExt, 2);
    payload += ",\"umidExterna\":" + String(hExt, 2);
  }
  payload += "}";

  Serial.println("Publicando via HTTPS:");
  Serial.println(payload);

  ledsEnviandoDados();

  if (WiFi.status() == WL_CONNECTED) {
    WiFiClientSecure client;
    client.setInsecure();
    HTTPClient https;

    https.begin(client, String("https://mytb.fabricadesoftware.ifc.edu.br/api/v1/") + token + "/telemetry");
    https.addHeader("Content-Type", "application/json");

    int httpResponseCode = https.POST(payload);
    Serial.print("📡 Resposta HTTP: ");
    Serial.println(httpResponseCode);

    if (httpResponseCode > 0) {
      ledsPadraoFuncionamento();
    } else {
      ledsSomenteUmSensor();
    }

    https.end();
  } else {
    Serial.println("❌ Wi-Fi desconectado.");
    ledsSemSensores();
  }
}

// === Setup principal ===
void setup() {
  Serial.begin(115200);
  pinMode(redLed, OUTPUT);
  pinMode(greenLed, OUTPUT);
  piscarLedsSincronizados(5, 200);

  WiFi.mode(WIFI_STA);
  delay(1000);

  WiFiManager wm;
  wm.setTimeout(180);
  if (!wm.autoConnect("ESP32_ConfigAP")) {
    Serial.println("⚠️ Falha na conexão. Reiniciando...");
    ledsSemSensores();
    delay(3000);
    ESP.restart();
  }

  Serial.println("✅ Wi-Fi conectado!");
  Serial.println(WiFi.localIP());

  inicializarSensoresComTolerancia();

  // Envio inicial
  enviarLeitura();
}

// === Loop principal ===
void loop() {
  unsigned long now = millis();
  if (now - lastMsg > interval) {
    lastMsg = now;
    enviarLeitura();
  }

  delay(1000);
}
