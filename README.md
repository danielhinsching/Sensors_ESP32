# 🌾 Fazenda IoT

Projeto baseado em ESP32 para monitoramento ambiental e envio de dados via MQTT para a plataforma ThingsBoard.

---

## 📦 Tecnologias e Bibliotecas

- **ESP32** (placa `esp32dev`)
- **PlatformIO + VS Code**
- **WiFiManager** – configuração facilitada de Wi-Fi
- **DHT.h** – leitura de temperatura e umidade
- **PubSubClient** – comunicação via MQTT
- **ThingsBoard** – dashboard IoT

---

## ⚙️ Esquema de Hardware

- **Sensor DHT11**: temperatura e umidade — pino `33`
- **Sensor de Gás**:
  - Analógico — pino `39`
  - Digital — pino `4`
- **LEDs**:
  - Vermelho — pino `32`
  - Verde — pino `26`
- **Relé** — pino `23`

---

## 🚀 Como usar

### 1. Clone o repositório

```bash
git clone https://github.com/ivoriegel/fazendaiot
cd fazendaiot
```

### 2. Abra com o PlatformIO

Abra a pasta do projeto no VS Code com a extensão PlatformIO instalada.

### 3. Instale as dependências

O PlatformIO cuida disso automaticamente com base no `platformio.ini`.

### 4. Configure o token do ThingsBoard

No arquivo `src/main.cpp`, substitua:

```cpp
const char *token = "token do dispositivo criado";
```

Pelo **token real** do seu dispositivo ThingsBoard.

### 5. Compile e envie para o ESP32

Com o ESP32 conectado, clique em "Upload" no PlatformIO ou rode:

```bash
pio run --target upload
```

### 6. Acompanhe pelo monitor serial

```bash
pio device monitor
```

---

## 📡 Telemetria enviada

O ESP32 envia os dados para:

```
v1/devices/me/telemetry
```

Exemplo de payload JSON publicado via MQTT:

```json
{
  "temperatura": 23.5,
  "umidade": 56.2,
  "gás": 374
}
```

---

## 🤝 Contribua

Contribuições são bem-vindas! Abra uma *issue* com bugs ou sugestões, ou envie um *pull request* com melhorias.

---

## 📜 Licença

Este projeto está licenciado sob os termos da licença MIT. Veja o arquivo `LICENSE` para mais detalhes.
