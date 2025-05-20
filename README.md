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

## 🐧 Como usar PlatformIO no Linux com VS Code

Se você está usando Linux e quer programar com ESP32 no VS Code usando o **PlatformIO**, siga o passo a passo abaixo. Ele foi feito pensando em quem **já usa VS Code, mas nunca usou o PlatformIO**.

---

### ✅ 1. Instale o PlatformIO via Marketplace do VS Code

Agora o PlatformIO está disponível normalmente no Marketplace:

1. Abra o **VS Code**
2. Vá até a aba de **Extensões (Ctrl+Shift+X)**
3. Pesquise por **PlatformIO IDE**
4. Clique em **Instalar**

---

### 🧱 2. Crie um novo projeto PlatformIO

1. Após instalar, clique no ícone do **formigueiro** no menu lateral esquerdo (ícone do PlatformIO).
2. Clique em **“New Project”** ou "Create New Project".
3. Dê um nome ao projeto (ex: `fazendaiot`)
4. Selecione a **placa ESP32 Dev Module**
5. Escolha o **framework Arduino**
6. Clique em **Finish**

📸 *[Insira aqui um print da tela de criação do projeto]*

---

### 🧾 3. Substitua ou renomeie o `.ino`

Se você já tinha um arquivo `.ino` (do Arduino IDE), faça o seguinte:

1. Mova o conteúdo do `.ino` para o arquivo `src/main.cpp`
2. Caso o `.ino` já esteja na pasta `src/`, **renomeie o arquivo** para `main.cpp`

> ⚠️ O PlatformIO exige que o arquivo principal esteja nomeado como `main.cpp` e localizado dentro da pasta `src/`.

---

### ⚙️ 4. Configure a placa, porta e monitor serial

Abra o arquivo `platformio.ini` e edite (ou confirme) o seguinte conteúdo:

```ini
[env:esp32dev]
platform = espressif32
board = esp32dev
framework = arduino
monitor_speed = 115200
upload_port = /dev/ttyUSB0
```

> 📝 A porta (`upload_port`) pode variar dependendo do seu sistema. Para descobrir, conecte o ESP32 e use `ls /dev/tty*` no terminal antes e depois.

---

### 🚀 5. Compile e envie o código

Com o ESP32 conectado, você pode:

- Clicar no **check (✔)** no menu inferior para **compilar**
- Clicar na **seta (→)** para **enviar o código**

Ou via terminal:

```bash
pio run --target upload
```

---

### 🔍 6. Monitor serial

Para visualizar os dados enviados pelo ESP32:

```bash
pio device monitor
```

---

### 🧩 7. Rodar upload e monitor juntos

Para enviar o código e já abrir o monitor serial no terminal:

```bash
pio run --target upload && pio device monitor
```

---

### 📌 Dicas úteis

- O terminal padrão do PlatformIO pode não abrir automaticamente — **use o terminal do VS Code (Ctrl+`) para executar os comandos manualmente.**
- Se você tiver problema com permissão na porta `/dev/ttyUSB0`, adicione seu usuário ao grupo `dialout`:

```bash
sudo usermod -aG dialout $USER
# Depois reinicie a máquina
```

- Verifique sempre se o cabo USB está funcionando para **dados** e não apenas **energia**.

---

Agora você já pode programar ESP32 com PlatformIO no Linux usando apenas o VS Code! 🚀


---

## 🤝 Contribua

Contribuições são bem-vindas! Abra uma *issue* com bugs ou sugestões, ou envie um *pull request* com melhorias.

---

## 📜 Licença

Este projeto está licenciado sob os termos da licença MIT. Veja o arquivo `LICENSE` para mais detalhes.
