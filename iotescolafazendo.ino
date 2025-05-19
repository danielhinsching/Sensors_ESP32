#incluir  <WiFi.h>
#include  <PubSubClient.h>
#incluir  <DHT.h>
#include  <WiFiManager.h>

#define  DHTTYPE DHT11
#define  DHTPIN  33
DHT dht ( DHTPIN, DHTTYPE ) ;

const  int gasAnalogPin = 39 ;
const  int gasDigitalPin = 4 ;
const  int redLed = 32 ;
const  int greenLed = 26 ;
const  int relePin = 23 ;

WiFiClient espClient;
Cliente PubSubClient ( espClient ) ;

unsigned  long lastMsg = 0 ;
const long interval = 900000;  // 15 minutos

// Quadro de Coisas
const  char * mqtt_server = "mytb.fabricadesoftware.ifc.edu.br" ;
const  int mqtt_port = 1883 ;
const  char * token = "token do dispositivo criado" ;   // Substitua pelo token do dispositivo no ThingsBoard
 configuração vazia_wifi ()  {
  Gerenciador de WiFi Gerenciador de WiFi;
  wifiManager . autoConnect ( " AutoConnectAP" ) ;
  Serial.println("WiFi conectado");
}

vazio  reconectar ()  {
  enquanto  ( ! cliente . conectado ())  {
    Serial.print("Tentando conectar ao MQTT...");
    se  ( cliente . conectar ( "ESP32Client" , token, NULL ))  {
      Serial.println("conectado");
    }  outro  {
      Serial.print("falhou, rc=");
      Serial . print ( cliente . estado ()) ;
      Serial.println(" tentando novamente em 5 segundos");
      atraso ( 5000 ) ;
    }
  }
}

 configuração vazia ()  {
  Serial . begin ( 115200 ) ;
  pinMode ( led vermelho, SAÍDA ) ;
  pinMode ( led verde, SAÍDA ) ;
  pinMode ( relePin, SAÍDA ) ;
  dht . começar () ;

  configuração_wifi () ;
  cliente . setServer ( mqtt_server, mqtt_port ) ;
}

 laço vazio ()  {
  se  ( ! cliente . conectado ())  {
    reconectar () ;
  }
  cliente . loop () ;

  unsigned  long agora = millis () ;
  se  ( agora - lastMsg > intervalo )  {
    lastMsg = agora;

    flutuar temp = dht . readTemperature () ;
    float hum = dht . readHumidity () ;
    int gasValue = analogRead ( gasAnalogPin ) ;

    // Formata os dados em JSON
    Carga útil da sequência de caracteres = "{" ;
    carga útil += "\"temperatura\":" + String ( temp ) + "," ;
    payload += "\"umidade\":" + String(hum) + ",";
    carga útil += "\"gás\":" + String ( gasValue ) ;
    carga útil += "}" ;

    Serial.println("Publicando: ");
    Serial . println ( carga útil ) ;

    cliente . publicar ( "v1/dispositivos/me/ telemetria" , carga útil . c_str ()) ;
  }
}
